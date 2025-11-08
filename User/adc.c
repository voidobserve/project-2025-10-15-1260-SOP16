#include "adc.h"
#include "my_config.h"

// 存放温度状态的变量
volatile u8 temp_status = TEMP_NORMAL;

volatile u16 adc_val_from_engine; // 存放 从发动机一侧 采集到的ad值
volatile u16 adc_val_from_knob;   // 存放 从旋钮一侧 采集到的ad值
volatile u16 adc_val_from_temp;   // 存放 从热敏电阻一侧 采集到的ad值
volatile u16 adc_val_from_fan;    // 存放 检测风扇一侧 采集到的ad值

volatile bit flag_tim_scan_fan_is_err = 0;      // 标志位，由定时器扫描并累计时间，表示当前风扇是否异常
volatile u8 cur_fan_status = FAN_STATUS_NORMAL; // 当前风扇状态

volatile u8 cur_adc2_status = ADC2_STATUS_NONE; // 状态机，表示当前adc2的状态

// volatile bit flag_is_pin_9_vol_bounce = 0; // 标志位，9脚电压是否发生了跳动

// adc相关的引脚配置
void adc_pin_config(void)
{
    // P30--8脚配置为模拟输入模式
    P3_MD0 |= GPIO_P30_MODE_SEL(0x3);

    // P27--9脚配置为模拟输入模式
    P2_MD1 |= GPIO_P27_MODE_SEL(0x3);

    // P31--7脚配置为模拟输入模式
    P3_PU &= ~(0x01 << 1); // 关闭上拉
    P3_PD &= ~(0x01 << 1); // 关闭下拉
    P3_MD0 |= GPIO_P31_MODE_SEL(0x3);

    // P13 -- 芯片的1脚，配置为模拟输入模式
    P1_PU &= ~(0x01 << 3);             // 关闭上拉
    P1_PD &= ~(0x01 << 3);             // 关闭下拉
    P1_MD0 |= GPIO_P13_MODE_SEL(0x03); // 模拟IO工作模式
}

// 切换adc采集的引脚，配置好adc
// 参数可以选择：
// ADC_SEL_PIN_GET_TEMP
// ADC_SEL_PIN_GET_VOL
// void adc_sel_pin(const u8 adc_sel)
// {
//     // 切换采集引脚时，把之前采集到的ad值清空
//     // adc0_val = 0;
//     static u8 last_adc_sel = 0;
//     if (last_adc_sel == adc_sel)
//     {
//         // 如果当前采集adc的引脚就是要配置的adc引脚，不用再继续配置，直接退出
//         return;
//     }

//     last_adc_sel = adc_sel;

//     ADC_CFG1 |= (0x0F << 3); // ADC时钟分频为16分频，为系统时钟/16
//     ADC_CFG2 = 0xFF;         // 通道0采样时间配置为256个采样时钟周期

//     switch (adc_sel)
//     {
//     case ADC_SEL_PIN_GET_TEMP: // 采集热敏电阻对应的电压的引脚（8脚）

//         // ADC配置
//         ADC_ACON1 &= ~(ADC_VREF_SEL(0x7) | ADC_EXREF_SEL(0) | ADC_INREF_SEL(0)); // 关闭外部参考电压，清除选择的参考电压
//         ADC_ACON1 |= ADC_VREF_SEL(0x6) |                                         // 选择内部参考电压VCCA
//                      ADC_TEN_SEL(0x3);                                           // 关闭测试信号
//         ADC_ACON0 = ADC_CMP_EN(0x1) |                                            // 打开ADC中的CMP使能信号
//                     ADC_BIAS_EN(0x1) |                                           // 打开ADC偏置电流能使信号
//                     ADC_BIAS_SEL(0x1);                                           // 偏置电流：1x

//         ADC_CHS0 = ADC_ANALOG_CHAN(0x18) | // 选则引脚对应的通道（0x18--P30）
//                    ADC_EXT_SEL(0x0);       // 选择外部通道

//         break;

//     case ADC_SEL_PIN_GET_VOL: // 检测回路电压的引脚（9脚）

//         // ADC配置
//         ADC_ACON1 &= ~(ADC_VREF_SEL(0x7) | ADC_EXREF_SEL(0x01)); // 关闭外部参考电压，清除选择的参考电压
//         // ADC_ACON1 |= ADC_VREF_SEL(0x6) |                                         // 选择内部参考电压VCCA
//         //              ADC_TEN_SEL(0x3);
//         ADC_ACON1 |= ADC_VREF_SEL(0x5) |   // 选择内部参考电压 4.2V (用户手册说未校准)
//                      ADC_TEN_SEL(0x3);     /* 关闭测试信号 */
//         ADC_ACON0 = ADC_CMP_EN(0x1) |      // 打开ADC中的CMP使能信号
//                     ADC_BIAS_EN(0x1) |     // 打开ADC偏置电流能使信号
//                     ADC_BIAS_SEL(0x1);     // 偏置电流：1x
//         ADC_CHS0 = ADC_ANALOG_CHAN(0x17) | // 选则引脚对应的通道（0x17--P27）
//                    ADC_EXT_SEL(0x0);       // 选择外部通道

//         break;

//     case ADC_SEL_PIN_P31: // P31、7脚，检测旋钮调光

//         // ADC配置
//         ADC_ACON1 &= ~(ADC_VREF_SEL(0x7)); // 关闭外部参考电压、清除选择的参考电压
//         ADC_ACON1 |= ADC_VREF_SEL(0x6) |   // 选择内部参考电压VCCA
//                      ADC_TEN_SEL(0x3);     // 关闭测试信号
//         ADC_ACON0 = ADC_CMP_EN(0x1) |      // 打开ADC中的CMP使能信号
//                     ADC_BIAS_EN(0x1) |     // 打开ADC偏置电流能使信号
//                     ADC_BIAS_SEL(0x1);     // 偏置电流：1x
//         ADC_CHS0 = ADC_ANALOG_CHAN(0x19) | // 选则引脚对应的通道（0x19--P31）
//                    ADC_EXT_SEL(0x0);       // 选择外部通道

//         break;

//     case ADC_SEL_PIN_FAN_DETECT: // P13 芯片的1脚，检测风扇是否异常

//         ADC_ACON1 &= ~(ADC_VREF_SEL(0x7) | ADC_EXREF_SEL(0x01) | ADC_INREF_SEL(0)); // 关闭外部参考电压，不选择外部参考，清除选择的参考电压
//         ADC_ACON1 |= ADC_VREF_SEL(0x6) |                                            // 选择内部参考电压VCCA
//                      ADC_TEN_SEL(0x3);                                              // 关闭测试信号
//         ADC_ACON0 = ADC_CMP_EN(0x1) |                                               // 打开ADC中的CMP使能信号
//                     ADC_BIAS_EN(0x1) |                                              // 打开ADC偏置电流能使信号
//                     ADC_BIAS_SEL(0x1);                                              // 偏置电流：1x

//         ADC_CHS0 = ADC_ANALOG_CHAN(0x0B) | // 选则引脚对应的通道（0x0B--P13）
//                    ADC_EXT_SEL(0x0);       // 选择外部通道

//         break;
//     }

//     ADC_CFG0 |= ADC_CHAN0_EN(0x1) | // 使能通道0转换
//                 ADC_EN(0x1);        // 使能A/D转换
//     // delay_ms(1);                    // 等待ADC稳定
//     // 官方的demo中提到等待20us以上
//     delay((u32)1450 / 2);
//     // delay((u32)1450 / 4);
// }

void adc_config(void)
{
    __EnableIRQ(ADC_IRQn); // 使能ADC中断
    IE_EA = 1;             // 使能总中断

    ADC_ACON1 &= ~(ADC_VREF_SEL(0x7) |   // 清除选择的参考电压
                   ADC_EXREF_SEL(0x01) | // 关闭外部参考电压
                   ADC_INREF_SEL(0x01)); // 关闭内部参考电压
    ADC_ACON1 |= ADC_VREF_SEL(0x6) |     // 选择内部参考电压VCCA （固定使用VCCA，不去修改）
                 ADC_TEN_SEL(0x3);       // 关闭测试信号
    ADC_ACON0 = ADC_CMP_EN(0x1) |        // 打开ADC中的CMP使能信号
                ADC_BIAS_EN(0x1) |       // 打开ADC偏置电流能使信号
                ADC_BIAS_SEL(0x1);       // 偏置电流：1x

    ADC_TRGS0 |= (0x07 << 4); // 通道 0DLY 的 ADC 时钟个数选择，配置为 4n+1，4 * 29 + 1
    ADC_CHS0 |= (0x01 << 6);  // 使能 通道 0DLY 功能

    ADC_TRGS1 |= (0x07 << 4); // 配置通道 1DLY 的 ADC 时钟个数选择，配置为 4n+1，4 * 29 + 1
    ADC_CHS1 |= (0x01 << 6);  // 使能通道 1DLY 功能

    ADC_TRGS2 |= (0x07 << 4); // 配置通道 2DLY 的 ADC 时钟个数选择，配置为 4n+1，4 * 29 + 1
    ADC_CHS2 |= (0x01 << 6);  // 使能通道 2DLY 功能

    ADC_CFG1 |= (0x0F << 3) | // ADC时钟分频为16分频，为系统时钟/16
                (0x01 << 2) | // ADC2 通道中断使能
                (0x01 << 1) | // ADC1 通道中断使能
                (0x01 << 0);  // ADC0 通道中断使能
    ADC_CFG2 = 0xFF;          // 通道0 采样时间配置为256个采样时钟周期
    ADC_CFG3 = 0xFF;          // 通道1 采样时间配置为256个采样时钟周期
    ADC_CFG4 = 0xFF;          // 通道2 采样时间配置为256个采样时钟周期

    adc0_channel_sel();
    adc1_channel_sel();
    adc2_channel_sel(ADC_SEL_PIN_NONE);

    ADC_CFG0 |= ADC_CHAN0_EN(0x01) |
                ADC_CHAN1_EN(0x01) |
                ADC_CHAN2_EN(0x01) |
                ADC_EN(0x01); // 使能adc

    delay_ms(1); // 等待adc稳定
}

// 配置adc0，adc0只选择检测发动机的通道
void adc0_channel_sel(void)
{
    ADC_CHS0 &= ~(ADC_EXT_SEL(0x01));  // 选择外部通道
    ADC_CHS0 &= ~(0x1F << 0);          // 清空 adc0 选择的模拟通道
    ADC_CHS0 |= ADC_ANALOG_CHAN(0x17); // adc0 选则引脚对应的通道（0x17--P27）
}

// 配置adc1 adc1只检测旋钮调光的通道
void adc1_channel_sel(void)
{
    ADC_CHS1 &= ~(ADC_EXT_SEL(0x01)); // 选择外部通道
    ADC_CHS1 &= ~(0x1F << 0);         // 清空 adc1 选择的模拟通道
    ADC_CHS1 = ADC_ANALOG_CHAN(0x19); // 选则引脚对应的通道（0x19--P31）
}

/**
 * @brief 配置adc2 adc2 要检测温度、风扇状态
 *      切换完成后，不能马上使用，要等adc稳定
 */
void adc2_channel_sel(u8 adc_sel_pin)
{
    static u8 last_adc_sel = 0;
    if (last_adc_sel == adc_sel_pin)
    {
        // 如果当前采集adc的引脚就是要配置的adc引脚，不用再继续配置，直接退出
        return;
    }

    last_adc_sel = adc_sel_pin;

    ADC_CHS2 &= ~(ADC_EXT_SEL(0x01)); // 选择外部通道
    ADC_CHS2 &= ~(0x1F << 0);         // 清空 adc2 选择的模拟通道

    switch (adc_sel_pin)
    {
    case ADC_SEL_PIN_GET_TEMP:
    {
        ADC_CHS2 |= 0x18 << 0;
    }
    break;
    case ADC_SEL_PIN_FAN_DETECT:
    {
        ADC_CHS2 |= 0x0B << 0;
    }
    break;

    default:
        break;
    }
}

// adc单次采集+转换（没有滤波）
// u16 adc_get_val_single(void)
// {
//     u16 adc_val = 0;
//     ADC_CFG0 |= ADC_CHAN0_TRG(0x1); // 触发ADC0转换
//     while (!(ADC_STA & ADC_CHAN0_DONE(0x1)))
//         ;                                            // 等待转换完成
//     adc_val = (ADC_DATAH0 << 4) | (ADC_DATAL0 >> 4); // 读取channel0的值
//     ADC_STA = ADC_CHAN0_DONE(0x1);                   // 清除ADC0转换完成标志位
//     return adc_val;
// }

// 获取一次adc采集+滤波后的值
// u16 adc_get_val(void)
// {
//     u8 i = 0; // adc采集次数的计数
//     volatile u16 g_temp_value = 0;
//     volatile u32 g_tmpbuff = 0;
//     volatile u16 g_adcmax = 0;
//     volatile u16 g_adcmin = 0xFFFF;

//     // 采集20次，去掉前两次采样，再去掉一个最大值和一个最小值，再取平均值
//     for (i = 0; i < 20; i++)
//     {
//         ADC_CFG0 |= ADC_CHAN0_TRG(0x1); // 触发ADC0转换
//         while (!(ADC_STA & ADC_CHAN0_DONE(0x1)))
//             ;                                                 // 等待转换完成
//         g_temp_value = (ADC_DATAH0 << 4) | (ADC_DATAL0 >> 4); // 读取 channel0 的值
//         ADC_STA = ADC_CHAN0_DONE(0x1);                        // 清除ADC0转换完成标志位

//         if (i < 2)
//             continue; // 丢弃前两次采样的
//         if (g_temp_value > g_adcmax)
//             g_adcmax = g_temp_value; // 最大
//         if (g_temp_value < g_adcmin)
//             g_adcmin = g_temp_value; // 最小

//         g_tmpbuff += g_temp_value;
//     }

//     g_tmpbuff -= g_adcmax;           // 去掉一个最大
//     g_tmpbuff -= g_adcmin;           // 去掉一个最小
//     g_temp_value = (g_tmpbuff >> 4); // 除以16，取平均值

//     return g_temp_value;
// }

// 从引脚上采集滤波后的电压值,函数内部会将采集到的ad转换成对应的电压值
u32 get_voltage_from_pin(void)
{  
    return (u32)adc_val_from_temp * 12 / 10;
}

// 温度检测功能
void temperature_scan(void)
{
    volatile u32 voltage = 0; // 存放adc采集到的电压，单位：mV

    // 如果已经超过75摄氏度且超过5min，不用再检测8脚的电压，等待用户排查原因，再重启（重新上电）
    if (TEMP_75_5_MIN == temp_status)
    {
        return;
    }

    {
        // 调用该函数一定次数之后，才进行温度检测，缩短主循环的执行周期
        static volatile u8 cnt = 0;
        cnt++;
        if (cnt < 100)
        {
            return;
        }

        cnt = 0;
    }
 
    voltage = get_voltage_from_pin(); // 得到热敏电阻上的电压
 

#if USE_MY_DEBUG
    // printf("PIN-8 voltage: %lu mV\n", voltage);
#endif // USE_MY_DEBUG

    // 如果之前的温度为正常，检测是否超过75摄氏度（±5摄氏度）
    // if (TEMP_NORMAL == temp_status && voltage < VOLTAGE_TEMP_75)
    if (TEMP_NORMAL == temp_status)
    {
        // 如果检测到温度大于75摄氏度（测得的电压值要小于75摄氏度对应的电压值）
        static volatile u8 cnt = 0;
        if (voltage < VOLTAGE_TEMP_75) // 如果检测到温度大于75摄氏度（测得的电压值要小于75摄氏度对应的电压值）
        {
            cnt++;
        }
        else
        {
            cnt = 0;
        }

        if (cnt >= 10)
        {
            cnt = 0; 
            temp_status = TEMP_75; // 状态标志设置为超过75摄氏度
        }
        else
        {
            temp_status = TEMP_NORMAL;
        }

        return; // 函数返回，让调节占空比的函数先进行调节
    }
    else if (TEMP_75 == temp_status)
    {
        // 如果之前的温度超过75摄氏度
        static bit tmr1_is_open = 0;

        if (0 == tmr1_is_open)
        {
            tmr1_is_open = 1;
            tmr1_cnt = 0;
            tmr1_enable(); // 打开定时器，开始记录是否大于75摄氏度且超过一定时间
        }

        // 如果超过75摄氏度并且过了5min，再检测温度是否超过75摄氏度
        if (tmr1_cnt >= (u32)TMR1_CNT_5_MINUTES)
        {
            static volatile u8 cnt = 0;

            if (voltage < VOLTAGE_TEMP_75) // 如果检测到温度大于75摄氏度（测得的电压值要小于75摄氏度对应的电压值）
            {
                cnt++;
            }
            else
            {
                cnt = 0;
            }

            if (cnt >= 10)
            {
                cnt = 0;
                temp_status = TEMP_75_5_MIN; // 状态标志设置为超过75摄氏度且超过5min
                tmr1_disable();              // 关闭定时器
                tmr1_cnt = 0;                // 清空时间计数值
                tmr1_is_open = 0;
            }
            else
            {
                temp_status = TEMP_75;
            }

            return;
        }
    }
}

// 根据温度（电压值扫描）或9脚的状态来设定占空比
void set_duty(void)
{
    // 如果温度正常，根据9脚的状态来调节PWM占空比
    if (TEMP_NORMAL == temp_status)
    {
        if (flag_is_time_to_check_engine)
        {
            flag_is_time_to_check_engine = 0;
            according_pin9_to_adjust_pwm();
        }
    }
    else if (TEMP_75 == temp_status)
    {
        limited_pwm_duty_due_to_temp = PWM_DUTY_50_PERCENT; // 将pwm占空比限制到最大占空比的 50%
    }
    else if (TEMP_75_5_MIN == temp_status)
    {
        limited_pwm_duty_due_to_temp = PWM_DUTY_25_PERCENT; // 将pwm占空比限制到最大占空比的 25%
    }
}

// volatile u16 adc_val_pin_9_filter_count = 0;
// u16 adc_val_pin_9_temp;
// 更新9脚采集的ad值
// void adc_update_pin_9_adc_val(void)
// {
//     adc_sel_pin(ADC_SEL_PIN_GET_VOL);
//     adc_val_pin_9 = adc_get_val();

//     // adc_val_pin_9 = 1000; // 测试时使用

// #if USE_MY_DEBUG // 打印从9脚采集到的ad值
//     // printf("adc_val_pin_9 %u\n", adc_val_pin_9);

//     // printf(",a=%u,", adc_val_pin_9);
// #endif // USE_MY_DEBUG // 打印从9脚采集到的ad值
// }

void fan_scan(void)
{
    u16 adc_val = adc_val_from_fan; // adc_val_from_fan 由adc中断触发

    /*
        1脚电压低于4.3V时，14，15脚输出25%占空比，
        1脚电压高于4.5V时，14，15脚输出100%占空比
    */
    if (FAN_STATUS_NORMAL == cur_fan_status)
    {
        if (adc_val <= ADC_VAL_WHEN_FAN_ERR)
        {
            flag_tim_scan_fan_is_err = 1; // 表示风扇异常，让定时器累计时间
        }
        else
        {
            // 风扇正常时，只要有一次ad值不满足异常的条件，便认为它是正常工作
            flag_tim_scan_fan_is_err = 0;
        }

        // 风扇正常工作，pwm正常输出
        limited_pwm_duty_due_to_fan_err = PWM_DUTY_100_PERCENT;
    }
    else // FAN_STATUS_ERROR == cur_fan_status
    {
        // 风扇异常时，检测到的ad值要与【风扇异常时对应的ad值】相隔一个死区，才认为风扇恢复正常
        if (adc_val >= ADC_VAL_WHEN_FAN_NORMAL)
        {
            flag_tim_scan_fan_is_err = 0; // 表示风扇正常
        }

        // 风扇工作异常，限制pwm输出，占空比不超过25%
        limited_pwm_duty_due_to_fan_err = PWM_DUTY_25_PERCENT;
    }
}

void ADC_IRQHandler(void) interrupt ADC_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(ADC_IRQn);

    // ---------------- 用户函数处理 -------------------

    if (ADC_STA & ADC_CHAN0_DONE(0x01))
    {
        ADC_STA |= ADC_CHAN0_DONE(0x01); // 清除ADC0转换完成标志位
        adc_val_from_engine = (ADC_DATAH0 << 4) | (ADC_DATAL0 >> 4);
    }

    if (ADC_STA & ADC_CHAN1_DONE(0x01))
    {
        ADC_STA |= ADC_CHAN1_DONE(0x01); // 清除ADC1转换完成标志位
        adc_val_from_knob = (ADC_DATAH1 << 4) | (ADC_DATAL1 >> 4);
    }

    if (ADC_STA & ADC_CHAN2_DONE(0x01))
    {
        ADC_STA |= ADC_CHAN2_DONE(0x01); // 清除ADC2转换完成标志位

        if (ADC2_STATUS_SEL_GET_TEMP == cur_adc2_status)
        {
            // 如果当前对应的是获取温度的adc通道
            adc_val_from_temp = (ADC_DATAH2 << 4) | (ADC_DATAL2 >> 4);
        }
        else if (ADC2_STATUS_SEL_FAN_DETECT == cur_adc2_status)
        {
            // 如果当前对应的是检测风扇的adc通道
            adc_val_from_fan = (ADC_DATAH2 << 4) | (ADC_DATAL2 >> 4);
        }
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(ADC_IRQn);
}
