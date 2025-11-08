/**
 ******************************************************************************
 * @file    main.c
 * @author  HUGE-IC Application Team
 * @version V1.0.0
 * @date    02-09-2022
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2021 HUGE-IC</center></h2>
 *
 * 版权说明后续补上
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "my_config.h"
#include "include.h"
#include <math.h>
#include <stdio.h>

float step = 70;
float mi; // 幂
// float rus; // 10的幂次方
// float r_ms = 0;
// #define USER_BAUD (115200UL)
// #define USER_UART_BAUD ((SYSCLK - USER_BAUD) / (USER_BAUD))

volatile bit flag_is_in_power_on; // 是否处于开机缓启动

#if USE_MY_DEBUG // 打印串口配置

#define UART0_BAUD 115200
#define USER_UART0_BAUD ((SYSCLK - UART0_BAUD) / (UART0_BAUD))
// 重写puchar()函数
char putchar(char c)
{
    while (!(UART0_STA & UART_TX_DONE(0x01)))
        ;
    UART0_DATA = c;
    return c;
}

void my_debug_config(void)
{
    // 作为发送引脚
    // P1_MD0 &= (GPIO_P13_MODE_SEL(0x3));
    // P1_MD0 |= GPIO_P13_MODE_SEL(0x1);            // 配置为输出模式
    // FOUT_S13 |= GPIO_FOUT_UART0_TX;              // 配置为UART0_TX
    // UART0_BAUD1 = (USER_UART0_BAUD >> 8) & 0xFF; // 配置波特率高八位
    // UART0_BAUD0 = USER_UART0_BAUD & 0xFF;        // 配置波特率低八位
    // UART0_CON0 = UART_STOP_BIT(0x0) |
    //              UART_EN(0x1); // 8bit数据，1bit停止位

    P0_MD0 &= ~(GPIO_P00_MODE_SEL(0x3));
    P0_MD0 |= GPIO_P00_MODE_SEL(0x1);            // 配置为输出模式
    FOUT_S00 |= GPIO_FOUT_UART0_TX;              // 配置为UART0_TX
    UART0_BAUD1 = (USER_UART0_BAUD >> 8) & 0xFF; // 配置波特率高八位
    UART0_BAUD0 = USER_UART0_BAUD & 0xFF;        // 配置波特率低八位
    UART0_CON0 = UART_STOP_BIT(0x0) |
                 UART_EN(0x1); // 8bit数据，1bit停止位
}
#endif // USE_MY_DEBUG // 打印串口配置

// 开机缓启动，调节占空比：
void adjust_pwm_duty_when_power_on(void)
{
    // if (jump_flag == 1)
    // {
    //     // break;
    //     return
    // }
    // if (c_duty < 6000)
    if (cur_pwm_channel_0_duty < MAX_PWM_DUTY &&
        cur_pwm_channel_1_duty < MAX_PWM_DUTY)
    {
        mi = (step - 1) / (253 / 3) - 1;
        step += 0.5;
        // cur_pwm_channel_0_duty = pow(5, mi) * 60; // C 库函数 double pow(double x, double y) 返回 x 的 y 次幂
        // cur_pwm_channel_1_duty = pow(5, mi) * 60; // C 库函数 double pow(double x, double y) 返回 x 的 y 次幂
        cur_pwm_channel_0_duty = pow(5, mi) * 60; // C 库函数 double pow(double x, double y) 返回 x 的 y 次幂
        cur_pwm_channel_1_duty = cur_pwm_channel_0_duty;
    }

    if (cur_pwm_channel_0_duty >= MAX_PWM_DUTY ||
        cur_pwm_channel_1_duty >= MAX_PWM_DUTY)
    {
        cur_pwm_channel_0_duty = MAX_PWM_DUTY;
        cur_pwm_channel_1_duty = MAX_PWM_DUTY;
    }
    // printf("c_duty %d\n",c_duty);

    // delay_ms(16); // 每16ms调整一次PWM的脉冲宽度 ---- 校验码A488对应的时间
    // delay_ms(11); // 16 * 0.666 约为10.656   ---- 校验码B5E3对应的时间
}

void main(void)
{
    // 看门狗默认打开, 复位时间2s
    WDT_KEY = WDT_KEY_VAL(0xDD); //  关闭看门狗 (如需配置看门狗请查看“WDT\WDT_Reset”示例)

    system_init();

    // 关闭HCK和HDA的调试功能
    WDT_KEY = 0x55;  // 解除写保护
    IO_MAP &= ~0x01; // 清除这个寄存器的值，实现关闭HCK和HDA引脚的调试功能（解除映射）
    WDT_KEY = 0xBB;  // 写一个无效的数据，触发写保护

#if USE_MY_DEBUG // 打印串口配置
    // 初始化打印

    my_debug_config();
    printf("sys reset\n");

    // P02 配置为输出模式
    P0_MD0 &= ~(0x03 << 4); // 清空对应的寄存器配置
    P0_MD0 |= 0x01 << 4;    // 输出模式
    FOUT_S02 = GPIO_FOUT_AF_FUNC;

    // P21 配置为输出模式
    P2_MD0 &= ~(0x03 << 2);
    P2_MD0 |= 0x01 << 2; // 输出模式
    FOUT_S21 = GPIO_FOUT_AF_FUNC;

    // 输出模式：
    // P1_MD0 &= (GPIO_P13_MODE_SEL(0x3));
    // P1_MD0 |= GPIO_P13_MODE_SEL(0x1); // 配置为输出模式
    // FOUT_S13 = GPIO_FOUT_AF_FUNC;     // 选择AF功能输出
#endif // 打印串口配置

#if 1
    adc_pin_config(); // 配置使用到adc的引脚
    adc_config();

    tmr0_config(); // 配置定时器，默认关闭
    pwm_init();    // 配置pwm输出的引脚
    tmr1_config();

    timer2_config();
    timer3_config(); // 要等adc完成初始化，再调用timer3的初始化

    rf_recv_init(); // rf功能初始化
    fan_ctl_config();
#endif

    limited_max_pwm_duty = MAX_PWM_DUTY;
    limited_pwm_duty_due_to_fan_err = MAX_PWM_DUTY;
    limited_pwm_duty_due_to_temp = MAX_PWM_DUTY;
    limited_pwm_duty_due_to_unstable_engine = MAX_PWM_DUTY;

// ===================================================================
#if 1 // 开机缓慢启动（PWM信号变化平缓）

    P14 = 0; // 16脚先输出低电平
    // c_duty = 0;
    cur_pwm_channel_0_duty = 0;
    cur_pwm_channel_1_duty = 0;
    flag_is_in_power_on = 1; // 表示到了开机缓启动
    // while (c_duty < 6000)
    // while (c_duty < limited_max_pwm_duty) // 当c_duty 大于 限制的最大占空比后，退出
    while (cur_pwm_channel_0_duty < limited_max_pwm_duty || /* 当 cur_pwm_channel_0_duty 大于 限制的最大占空比后，退出 */
           cur_pwm_channel_1_duty < limited_max_pwm_duty)   /* 当 cur_pwm_channel_1_duty 大于 限制的最大占空比后，退出 */
    {
        // adc_update_pin_9_adc_val();        // 采集并更新9脚的ad值
        update_max_pwm_duty_coefficient(); // 更新当前的最大占空比

#if USE_MY_DEBUG // 直接打印0，防止在串口+图像上看到错位

        // printf(",b=0,"); // 防止在串口图像错位

#endif

        if (flag_is_pwm_sub_time_comes) // pwm递减时间到来（该标志位主要用在发动机功率不稳定检测中）
        {
            flag_is_pwm_sub_time_comes = 0;

            /*
                只要有一次跳动，退出开机缓启动(改成等到变为 limited_max_pwm_duty 再退出)，
                由于 adjust_duty 初始值为 MAX_PWM_DUTY ，直接退出会直接设置占空比为 adjust_duty 对应的值，
                会导致灯光闪烁一下

                目前没有使用提前退出开机缓启动的功能
            */
            // if (adc_val_pin_9 >= ADC_VAL_WHEN_UNSTABLE)
            // {
            //     // if (c_duty >= PWM_DUTY_100_PERCENT)
            //     // if (c_duty >= limited_max_pwm_duty)
            //     if (cur_pwm_channel_0_duty >= limited_max_pwm_duty &&
            //         cur_pwm_channel_1_duty >= limited_max_pwm_duty)
            //     {
            //         // adjust_duty = c_duty;
            //         break;
            //     }
            // }
        }

        if (flag_time_comes_during_power_on) // 如果调节时间到来 -- 13ms
        {
            flag_time_comes_during_power_on = 0;
            adjust_pwm_duty_when_power_on();
        }

        set_pwm_channel_0_duty(cur_pwm_channel_0_duty);
        set_pwm_channel_1_duty(cur_pwm_channel_1_duty);
        // set_pwm_duty(); // 将 c_duty 写入pwm对应的寄存器
        // set_p15_pwm_duty(c_duty);

#if USE_MY_DEBUG
        // printf("power_on_duty %u\n", c_duty);
#endif //  USE_MY_DEBUG
    }
#endif // 开机缓慢启动（PWM信号变化平缓）

    // 缓启动后，立即更新 adjust_duty 的值：
    adjust_pwm_channel_0_duty = cur_pwm_channel_0_duty;
    adjust_pwm_channel_1_duty = cur_pwm_channel_1_duty;
    flag_is_in_power_on = 0; // 表示退出了开机缓启动
    // ===================================================================

    while (1)
    {
#if 1
        update_max_pwm_duty_coefficient(); // 根据当前旋钮的挡位，限制能调节到的最大的pwm占空比
        temperature_scan();                // 检测热敏电阻一端的电压值
        fan_scan();                        // 检测风扇的状态是否异常，并根据结果来限制pwm占空比
        set_duty();                        // 设定到要调节到的脉宽 (设置adjust_duty)

        // according_pin9_to_adjust_pin16();  // 根据9脚的电压来设定16脚的电平
        P14 = 0;

        if (flag_is_rf_enable) // 如果使能了rf遥控器的功能
        {
            key_driver_scan(&rf_key_para);
            rf_key_handle();
        }

        {
            // 如果 expect_adjust_pwm_channel_x_duty 有变化，可以在这里修改 adjust_pwm_channel_x_duty
            adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(expect_adjust_pwm_channel_0_duty);
            adjust_pwm_channel_1_duty = get_pwm_channel_x_adjust_duty(expect_adjust_pwm_channel_1_duty);
        }

        // 风扇控制：
        fan_ctl();
#endif

        // P02 = ~P02; // 测试主循环一轮所需时间

        // 测试用：
        // {
        //     static u16 cnt = 0;
        //     cnt++;

        //     // if (cnt >= 10)
        //     if (cnt >= 100)
        //     {
        //         cnt = 0;
        //         // printf("expect_adjust_pwm_channel_0_duty: %u\n", expect_adjust_pwm_channel_0_duty);
        //         // printf("expect_adjust_pwm_channel_1_duty: %u\n", expect_adjust_pwm_channel_1_duty);
        //         // printf("adjust_pwm_channel_0_duty: %u\n", adjust_pwm_channel_0_duty);
        //         // printf("adjust_pwm_channel_1_duty: %u\n", adjust_pwm_channel_1_duty);
        //         // printf("cur_pwm_channel_0_duty: %u\n", cur_pwm_channel_0_duty);
        //         // printf("cur_pwm_channel_1_duty: %u\n", cur_pwm_channel_1_duty);

        //         // printf("__LINE__ %u\n", __LINE__);
        //         // printf("val %u\n", ADC_OVER_DRIVE_VAL);
        //         // printf("val %u\n", (u16)adc_val_from_fan);
        //         // printf("val %u\n", (u16)adc_val_from_temp);
        //         // printf("val %u\n", (u16)adc_val_from_knob);
        //         // printf("val %u\n", (u16)adc_val_from_engine);

        //         // P02 = ~P02;
        //     }
        // }
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2022 HUGE-IC ***** END OF FILE *****/
