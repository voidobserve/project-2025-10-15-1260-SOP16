#include "pwm.h"
#include "time0.h"

// 由温度限制的PWM占空比 （对所有PWM通道都生效，默认为最大占空比）
volatile u16 limited_pwm_duty_due_to_temp = MAX_PWM_DUTY;
// 由于发动机不稳定，而限制的可以调节到的占空比（对所有PWM通道都生效，默认为最大占空比）
volatile u16 limited_pwm_duty_due_to_unstable_engine = MAX_PWM_DUTY;
// 由于风扇异常，限制的可以调节到的最大占空比（对所有PWM通道都生效，默认为最大占空比）
volatile u16 limited_pwm_duty_due_to_fan_err = MAX_PWM_DUTY;

volatile u16 cur_pwm_channel_0_duty;                          // 当前设置的、 pwm_channle_0 的占空比（只有遥控器指定要修改它的值或是定时器缓慢调节，才会被修改）
volatile u16 expect_adjust_pwm_channel_0_duty = MAX_PWM_DUTY; // 存放期望调节到的 pwm_channle_0 占空比
volatile u16 adjust_pwm_channel_0_duty = MAX_PWM_DUTY;        // pwm_channle_0 要调整到的占空比

volatile u16 cur_pwm_channel_1_duty;                          // 当前设置的第二路PWM的占空比（只有遥控器指定要修改它的值或是定时器缓慢调节，才会被修改）
volatile u16 expect_adjust_pwm_channel_1_duty = MAX_PWM_DUTY; // 存放期望调节到的 pwm_channle_1 占空比
volatile u16 adjust_pwm_channel_1_duty = MAX_PWM_DUTY;        // pwm_channle_1 要调整到的占空比

#define STMR0_PEROID_VAL (SYSCLK / 8000 - 1)
#define STMR1_PEROID_VAL (SYSCLK / 8000 - 1)
void pwm_init(void)
{
    STMR_CNTCLR |= STMR_0_CNT_CLR(0x1); // 清空计数值

    STMR0_PSC = STMR_PRESCALE_VAL(0x07);                        // 预分频
    STMR0_PRH = STMR_PRD_VAL_H((STMR0_PEROID_VAL >> 8) & 0xFF); // 周期值
    STMR0_PRL = STMR_PRD_VAL_L((STMR0_PEROID_VAL >> 0) & 0xFF);
    STMR0_CMPAH = STMR_CMPA_VAL_H(((0) >> 8) & 0xFF); // 比较值
    STMR0_CMPAL = STMR_CMPA_VAL_L(((0) >> 0) & 0xFF); // 比较值
    STMR_PWMVALA |= STMR_0_PWMVALA(0x1);

    STMR_CNTMD |= STMR_0_CNT_MODE(0x1); // 连续计数模式
    STMR_LOADEN |= STMR_0_LOAD_EN(0x1); // 自动装载使能
    STMR_CNTCLR |= STMR_0_CNT_CLR(0x1); //
    STMR_CNTEN |= STMR_0_CNT_EN(0x1);   // 使能
    STMR_PWMEN |= STMR_0_PWM_EN(0x1);   // PWM输出使能

    P1_MD1 &= ~GPIO_P16_MODE_SEL(0x03); // P16 14脚
    P1_MD1 |= GPIO_P16_MODE_SEL(0x01);
    P1_MD1 &= ~GPIO_P14_MODE_SEL(0x03); // P14 16脚
    P1_MD1 |= GPIO_P14_MODE_SEL(0x01);
    FOUT_S14 = GPIO_FOUT_AF_FUNC;      // AF功能输出
    FOUT_S16 = GPIO_FOUT_STMR0_PWMOUT; // stmr0_pwmout

    // P15 15脚 作为第2路PWM输出
    STMR_CNTCLR |= STMR_1_CNT_CLR(0x1);                         // 清空计数值
    STMR1_PSC = STMR_PRESCALE_VAL(0x07);                        // 预分频
    STMR1_PRH = STMR_PRD_VAL_H((STMR1_PEROID_VAL >> 8) & 0xFF); // 周期值
    STMR1_PRL = STMR_PRD_VAL_L((STMR1_PEROID_VAL >> 0) & 0xFF);
    STMR1_CMPAH = STMR_CMPA_VAL_H(((0) >> 8) & 0xFF); // 比较值 (清空比较值)
    STMR1_CMPAL = STMR_CMPA_VAL_L(((0) >> 0) & 0xFF); // 比较值
    STMR_PWMVALA |= STMR_1_PWMVALA(0x1);              // STMR1 PWM输出值 ( 0x1:计数CNT大于等于比较值A,PWM输出1,小于输出0 )

    STMR_CNTMD |= STMR_1_CNT_MODE(0x1); // 连续计数模式
    STMR_LOADEN |= STMR_1_LOAD_EN(0x1); // 自动装载使能
    STMR_CNTCLR |= STMR_1_CNT_CLR(0x1); //
    STMR_CNTEN |= STMR_1_CNT_EN(0x1);   // 使能
    STMR_PWMEN |= STMR_1_PWM_EN(0x1);   // PWM输出使能

#if USE_MY_TEST_PIN
    P0_MD1 &= ~GPIO_P05_MODE_SEL(0x03); // 用开发板上的 p05
    P0_MD1 |= GPIO_P05_MODE_SEL(0x01);  // 输出模式
    FOUT_S05 = GPIO_FOUT_STMR1_PWMOUT;  // 选择stmr1_pwmout
#else
    P1_MD1 &= ~GPIO_P15_MODE_SEL(0x03); // P15 15脚
    P1_MD1 |= GPIO_P15_MODE_SEL(0x01);  // 输出模式
    FOUT_S15 = GPIO_FOUT_STMR1_PWMOUT;  // 选择stmr1_pwmout
#endif //  #if USE_MY_TEST_PIN
}

// 设置通道0的占空比
void set_pwm_channel_0_duty(u16 channel_duty)
{
    STMR0_CMPAH = STMR_CMPA_VAL_H(((channel_duty) >> 8) & 0xFF); // 比较值
    STMR0_CMPAL = STMR_CMPA_VAL_L(((channel_duty) >> 0) & 0xFF); // 比较值
    STMR_LOADEN |= STMR_0_LOAD_EN(0x1);                          // 自动装载使能
}

// 设置通道1的占空比
void set_pwm_channel_1_duty(u16 channel_duty)
{
    STMR1_CMPAH = STMR_CMPA_VAL_H(((channel_duty) >> 8) & 0xFF); // 比较值
    STMR1_CMPAL = STMR_CMPA_VAL_L(((channel_duty) >> 0) & 0xFF); // 比较值
    STMR_LOADEN |= STMR_1_LOAD_EN(0x1);                          // 自动装载使能
}




// 根据9脚的电压来设定16脚的电平（过压保护）
// void according_pin9_to_adjust_pin16(void)
// {
//     // 当9脚电压高于 3.6 V时，16脚输出1KHz 高电平,用于控制Q2的导通（用于关机）。
//     // if (adc_val_pin_9 >= 3511)
//     // {
//     //     P14 = 1;
//     // }
//     // else if (adc_val_pin_9 <= 3511 - 40)
//     {
//         P14 = 0;
//     }
// }

/**
 * @brief 获取第一路PWM的运行状态
 *
 * @return u8 0--pwm关闭，1--pwm开启
 */
u8 get_pwm_channel_0_status(void)
{
    if (STMR_PWMEN & 0x01) // 如果pwm0使能
    {
        return 1;
    }
    else // 如果pwm0未使能
    {
        return 0;
    }
}

/**
 * @brief 获取第二路PWM的运行状态
 *
 * @return u8 0--pwm关闭，1--pwm开启
 */
u8 get_pwm_channel_1_status(void)
{
    if (STMR_PWMEN & (0x01 << 1)) // 如果pwm1使能
    {
        return 1;
    }
    else // 如果 pwm 未使能
    {
        return 0;
    }
}

void pwm_channel_0_enable(void)
{
    // 要先使能PWM输出，在配置IO，否则在逻辑分析仪上看会有个缺口
    STMR_PWMEN |= 0x01;                // 使能PWM0的输出
    FOUT_S16 = GPIO_FOUT_STMR0_PWMOUT; // stmr0_pwmout
}

void pwm_channel_0_disable(void)
{
    // 直接输出0%的占空比，可能会有些跳动，需要将对应的引脚配置回输出模式
    STMR_PWMEN &= ~0x01;          // 不使能PWM0的输出
    FOUT_S16 = GPIO_FOUT_AF_FUNC; //
    P16 = 1;                      // 高电平为关灯
}

void pwm_channel_1_enable(void)
{
    // 要先使能PWM输出，在配置IO，否则在逻辑分析仪上看会有个缺口
    STMR_PWMEN |= 0x01 << 1; // 使能PWM1的输出

#if USE_MY_TEST_PIN
    FOUT_S05 = GPIO_FOUT_STMR1_PWMOUT; // stmr1_pwmout
#else
    FOUT_S15 = GPIO_FOUT_STMR1_PWMOUT; // stmr1_pwmout
#endif
}

void pwm_channel_1_disable(void)
{
    // 直接输出0%的占空比，可能会有些跳动，需要将对应的引脚配置回输出模式
    STMR_PWMEN &= ~(0x01 << 1); // 不使能PWM1的输出

#if USE_MY_TEST_PIN
    FOUT_S05 = GPIO_FOUT_AF_FUNC; //;
    P05 = 1;                      // 高电平为关灯
#else
    FOUT_S15 = GPIO_FOUT_AF_FUNC; //
    P15 = 1;                      // 高电平为关灯
#endif
}

/**
 * @brief 根据传参，加上线控调光的限制、温度过热限制、风扇工作异常限制，
 *          计算最终的目标占空比（对所有pwm通道都有效）
 *
 * @attention 如果反复调用 adjust_pwm_channel_x_duty = get_pwm_channel_x_adjust_duty(adjust_pwm_channel_x_duty);
 *              会导致 adjust_pwm_channel_x_duty 越来越小
 *
 * @param pwm_adjust_duty 传入的目标占空比（非最终的目标占空比） expect_adjust_pwm_channel_x_duty
 *
 * @return u16 最终的目标占空比
 */
u16 get_pwm_channel_x_adjust_duty(u16 pwm_adjust_duty)
{
    // 存放函数的返回值 -- 最终的目标占空比
    // 根据设定的目标占空比，更新经过旋钮限制之后的目标占空比：
    u16 tmp_pwm_duty = (u32)pwm_adjust_duty * limited_max_pwm_duty / MAX_PWM_DUTY; // pwm_adjust_duty * 旋钮限制的占空比系数

    // 温度、发动机异常功率不稳定、风扇异常，都是强制限定占空比

    // 判断经过旋钮限制之后的占空比 会不会 大于 温度过热之后限制的占空比
    if (tmp_pwm_duty >= limited_pwm_duty_due_to_temp)
    {
        tmp_pwm_duty = limited_pwm_duty_due_to_temp;
    }

    // 如果限制之后的占空比 大于 由于发动机不稳定而限制的、可以调节的最大占空比
    if (tmp_pwm_duty >= limited_pwm_duty_due_to_unstable_engine)
    {
        tmp_pwm_duty = limited_pwm_duty_due_to_unstable_engine;
    }

    // 如果限制之后的占空比 大于 由于风扇异常，限制的可以调节到的最大占空比
    if (tmp_pwm_duty >= limited_pwm_duty_due_to_fan_err)
    {
        tmp_pwm_duty = limited_pwm_duty_due_to_fan_err;
    }

    return tmp_pwm_duty; // 返回经过线控调光限制之后的、最终的目标占空比
}

// 更新 pwm_channel_0 待调整的占空比
// void update_pwm_channel_0_adjust_duty(void)
// {
//     adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(expect_adjust_pwm_channel_0_duty);
// }
