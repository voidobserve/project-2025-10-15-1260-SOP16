#include "my_config.h"
#include "engine.h"

/*
    滤波、判断电压是否有跳动，一段时间内电压浮动过大，
    所使用到的这些变量
*/
#define FILTER_BUFF_SIZE 32
#define FILTER_BUFF_2_SIZE 270 // 270 -- 对应 5.83ms 执行一次的函数 according_pin9_to_adjust_pwm，时间越短，数组需要加大
// #define FILTER_BUFF_2_SIZE 540 // 540 -- 对应 2.93ms 执行一次的函数 according_pin9_to_adjust_pwm，时间越短，数组需要加大
static volatile u16 t_count = 0;
static volatile u16 t_adc_max = 0;    // 存放一段时间内采集到的最大ad值
static volatile u16 t_adc_min = 4096; // 存放一段时间内采集到的最小ad值
static volatile u8 over_drive_status = 0;
#define OVER_DRIVE_RESTART_TIME (30)

static volatile u16 filter_buff[FILTER_BUFF_SIZE] = {
    0xFFFF,
};

// static volatile u16 filter_buff_2[270] = {0}; // 用于滤波的数组 (对应 5.83ms 执行一次的函数 according_pin9_to_adjust_pwm )
// static volatile u16 filter_buff_2[25] = {0}; // 用于滤波的数组
static volatile u16 filter_buff_2[FILTER_BUFF_2_SIZE]; // 用于滤波的数组
static volatile u16 filter_buff_2_index;               // 用于记录滤波的数组下标

volatile bit flag_is_time_to_check_engine; // 标志位，是否到时间去调用检测发动机功率的函数

/*
    电源电压低于170V-AC,启动低压保护，电源电压高于170V-AC，关闭低压保护
    温度正常，才会进入到这里
    注意，每次调用到该函数，应该在5.75ms左右，检测发动机功率不稳定的原理是检测频率，
    如果不在5.75ms附近，可能会导致检测不准确，检测不到发动机功率不稳定
*/
void according_pin9_to_adjust_pwm(void)
{
    static volatile u8 filter_buff_index = 0;
    static volatile u8 flag_is_sub_power = 0;  // 标志位，是否要连续减功率（状态机）
    static volatile u8 flag_is_sub_power2 = 0; // 标志位，是否要连续减功率
    static volatile bit flag_is_add_power = 0; // 标志位，是否要连续增功率

    volatile u32 adc_pin_9_avg = 0; // 存放平均值
    volatile u16 adc_val = adc_val_from_engine; // adc_val_from_engine 由adc中断更新

    if (filter_buff[0] == 0xFFFF) // 如果是第一次检测，让数组内所有元素都变为第一次采集的数据，方便快速作出变化
    {
        u16 i = 0;
        for (; i < ARRAY_SIZE(filter_buff); i++)
        {
            filter_buff[i] = adc_val;
        }

        for (i = 0; i < ARRAY_SIZE(filter_buff_2); i++)
        {
            filter_buff_2[i] = adc_val;
        }
    }
    else
    {
        u16 temp = filter_buff[filter_buff_index];
        temp += adc_val;
        temp >>= 1;
        filter_buff[filter_buff_index] = temp;
        filter_buff_index++;
        if (filter_buff_index >= ARRAY_SIZE(filter_buff))
        {
            filter_buff_index = 0;
        }
    }

    { // 取出数组内的数据，计算平均值
        u16 i = 0;
        for (; i < ARRAY_SIZE(filter_buff); i++)
        {
            adc_pin_9_avg += filter_buff[i];
        }

        // adc_pin_9_avg /= ARRAY_SIZE(filter_buff);
        adc_pin_9_avg >>= 5; // 相当于 /= 32
    } // 取出数组内的数据，计算平均值

    // 在前面滤波的基础上再进行一次滤波
    filter_buff_2[filter_buff_2_index] = adc_pin_9_avg;
    filter_buff_2_index++;
    if (filter_buff_2_index >= ARRAY_SIZE(filter_buff_2))
    {
        filter_buff_2_index = 0;
    }

    {
        u16 i = 0;
        t_adc_max = 0;
        t_adc_min = 4096;
        for (; i < ARRAY_SIZE(filter_buff_2); i++)
        {
            if (filter_buff_2[i] > t_adc_max)
            {
                t_adc_max = filter_buff_2[i];
            }
            if (filter_buff_2[i] < t_adc_min)
            {
                t_adc_min = filter_buff_2[i];
            }
            if ((t_adc_max - t_adc_min) > ADC_OVER_DRIVE_VAL)
            { // 电压波动
                over_drive_status = OVER_DRIVE_RESTART_TIME;
            }
            else
            {
                if (over_drive_status)
                    over_drive_status--;
            }
        }

        // MY_DEBUG:
        // {
        //     // 根据发动机不稳定降功率的功能正常时，测得是5.83ms执行一次，每100次打印一次，平均耗时是583ms
        //     // 如果循环大于5.83ms，在客户那里测试好像功能也正常，客户没有进一步反馈
        //     static u8 cnt = 0;
        //     cnt++;
        //     if (cnt >= 100)
        //     {
        //         cnt = 0;
        //         printf("__LINE__ %u\n", __LINE__);
        //     }
        // }
    }

    if (adc_pin_9_avg >= (ADC_VAL_170VAC + ADC_DEAD_ZONE_NEAR_170VAC) ||
        (flag_is_add_power && adc_pin_9_avg > (ADC_VAL_170VAC + ADC_DEAD_ZONE_NEAR_170VAC))) // 大于 170VAC
    {
        // 大于170VAC，恢复100%占空比，但是优先级比 "9脚电压检测到发送机功率不稳定，进而降功率" 低
        flag_is_sub_power = 0;
        flag_is_sub_power2 = 0;
        flag_is_add_power = 1;

        if (over_drive_status == OVER_DRIVE_RESTART_TIME) // 9脚电压超过不稳定阈值对应的电压
        {
            over_drive_status -= 1;
            if (limited_pwm_duty_due_to_unstable_engine > PWM_DUTY_50_PERCENT)
            {
                limited_pwm_duty_due_to_unstable_engine -= 1;
            }

            if (limited_pwm_duty_due_to_unstable_engine < PWM_DUTY_50_PERCENT)
            {
                limited_pwm_duty_due_to_unstable_engine = PWM_DUTY_50_PERCENT;
            }
        }
        else if (over_drive_status == 0)
        {
            // 未满载 pwm++
            if (flag_is_pwm_add_time_comes) // pwm占空比递增时间到来
            {
                flag_is_pwm_add_time_comes = 0;
                if (limited_pwm_duty_due_to_unstable_engine < PWM_DUTY_100_PERCENT)
                {
                    limited_pwm_duty_due_to_unstable_engine++;
                }
            }
        }
    }
    else if (adc_pin_9_avg > (ADC_VAL_LOWER_THAN_170VAC) &&
             (adc_pin_9_avg <= (ADC_VAL_170VAC) || flag_is_sub_power == 4)) // 小于 170VAC
    {
        // 锁定最大的占空比为50%，并且给相应标志位置一
        if (flag_is_pwm_sub_time_comes) // pwm占空比递减时间到来
        {
            flag_is_pwm_sub_time_comes = 0;

            if (flag_is_sub_power < 4)
            {
                flag_is_sub_power++;
            }

            flag_is_sub_power2 = 0;
            flag_is_add_power = 0;

            if (limited_pwm_duty_due_to_unstable_engine > PWM_DUTY_50_PERCENT)
            {
                limited_pwm_duty_due_to_unstable_engine -= 2;
            }
            else if (limited_pwm_duty_due_to_unstable_engine < PWM_DUTY_50_PERCENT)
            {
                limited_pwm_duty_due_to_unstable_engine++;
            }
            else
            {
                limited_pwm_duty_due_to_unstable_engine = PWM_DUTY_50_PERCENT;
            }
        }
    }
    else if (adc_pin_9_avg <= (ADC_VAL_LOWER_THAN_170VAC) || (flag_is_sub_power2)) // 小于 170VAC
    {
        // 锁定最大的占空比为50%，并且给相应标志位置一
        if (flag_is_pwm_sub_time_comes) // pwm占空比递减时间到来
        {
            flag_is_pwm_sub_time_comes = 0;

            flag_is_sub_power2 = 1;
            flag_is_sub_power = 0;
            flag_is_add_power = 0;

            if (limited_pwm_duty_due_to_unstable_engine > PWM_DUTY_30_PERCENT)
            {
                limited_pwm_duty_due_to_unstable_engine -= 2;
            }
            else
            {
                limited_pwm_duty_due_to_unstable_engine = PWM_DUTY_30_PERCENT;
            }
        }
    }
}