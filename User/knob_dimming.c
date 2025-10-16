#include "knob_dimming.h" // 旋钮调光头文件

volatile u16 limited_max_pwm_duty = MAX_PWM_DUTY; // 存放经过旋钮限制之后的最大占空比（对所有pwm通道有效）
// volatile u16 limited_adjust_pwm_duty;  // 存放旋钮限制之后的，待调整的占空比值

// 根据旋钮，限制当前的最大占空比
// 会更新 limited_max_pwm_duty 这个全局变量
void update_max_pwm_duty_coefficient(void)
{
    /*
        标志位，上一次限制的最大占空比是否为0，
        如果为0，需要等到检测的ad值超过20%、30%......再使能旋钮的功能，否则不使能
        相当于给关机和开机之间划一个较大的死区
    */
    static bit flag_is_last_limited_equal_zero = 0;

    // static u16 last_limited_max_pwm_duty = MAX_PWM_DUTY;

    volatile u16 adc_val = 0;
    adc_sel_pin(ADC_SEL_PIN_P31);
    adc_val = adc_get_val();

    // limited_max_pwm_duty = (u32)adjust_duty * adc_val / 4096; // 会出现指数级下降，因为adjust_duty一直在变化
    // limited_max_pwm_duty = (u32)MAX_PWM_DUTY * adc_val / 4096; // 分级太多，加上抖动和迅速变化，会导致灯光闪烁

#if 1

    // 它闪灯可能是因为功率太低，也可能是检测脚的波动，导致单片机触发了频繁开灯关灯的操作
    // 方法1：试着加大调光那里的电容，提供一个缓冲
    // 方法2：单片机检测到关灯之后，再检测到旋钮调到20%、30%之后再开灯

    if (0 == flag_is_last_limited_equal_zero) // 如果上一次限制的最大占空比不是0，则正常地根据旋钮来限制最大占空比
    {
        // 下面条件语句块内的值和判断条件中的值不一样，不过影响不大
        // ad值是假设参考电压VCC为4.87V，计算得出
        if (adc_val <= KNOB_DIMMING_MIN_ADC_VAL) // MIN 0%  0.57V
        {
            limited_max_pwm_duty = 0;
        }
        // else if (adc_val <= 1278) // 20% 1.52V
        else if (adc_val <= KNOB_DIMMING_ADC_VAL_20_PERCENT) // 20% 1.52V /* 这里用5V作为参考电压 */
        {
            // 计算采集到的ad值所占20%对应的ad值的占比，再乘以 20%的占空比
            /*
                计算验证，这里的ad值如果只相差1，占空比的值也只相差1
            */
            // limited_max_pwm_duty = (u32)adc_val * MAX_PWM_DUTY * 2 / 10 / KNOB_DIMMING_ADC_VAL_20_PERCENT;
            // 旧版本这里最低只能调节到5%，现在让它能够调节到5%及以下:
            limited_max_pwm_duty = ((u32)adc_val - KNOB_DIMMING_MIN_ADC_VAL) * MAX_PWM_DUTY * 2 / 10 / KNOB_DIMMING_ADC_VAL_20_PERCENT;
        }
        // else if (adc_val <= 2498) // 40% 2.97V
        else if (adc_val <= KNOB_DIMMING_ADC_VAL_40_PERCENT) // 40% 2.79V /* 这里用5V作为参考电压 */
        {
            /*
                计算验证，这里的ad值如果只相差1，占空比的值也只相差1
            */
            limited_max_pwm_duty = (u32)adc_val * MAX_PWM_DUTY * 4 / 10 / KNOB_DIMMING_ADC_VAL_40_PERCENT;
        }
        // else if (adc_val <= 2641) // 50% 3.14V
        else if (adc_val <= KNOB_DIMMING_ADC_VAL_50_PERCENT) // 50% 3.46V /* 这里用5V作为参考电压 */
        {
            limited_max_pwm_duty = (u32)adc_val * MAX_PWM_DUTY / 2 / KNOB_DIMMING_ADC_VAL_50_PERCENT;
        }
        // else if (adc_val <= 3482) // 60% 4.14V
        else if (adc_val <= KNOB_DIMMING_ADC_VAL_60_PERCENT) // 60% 3.98V /* 这里用5V作为参考电压 */
        {
            limited_max_pwm_duty = (u32)adc_val * MAX_PWM_DUTY * 6 / 10 / KNOB_DIMMING_ADC_VAL_60_PERCENT;
        }
        // else if (adc_val <= 4087) // 80% 4.86V
        else if (adc_val <= KNOB_DIMMING_MAX_ADC_VAL) //
        {
            limited_max_pwm_duty = (u32)adc_val * MAX_PWM_DUTY * 8 / 10 / KNOB_DIMMING_MAX_ADC_VAL;
        }
        else
        {
            // limited_max_pwm_duty = (u32)adc_val * MAX_PWM_DUTY / 4095;
            limited_max_pwm_duty = MAX_PWM_DUTY;
        }

        // limited_adjust_pwm_duty = (u32)adjust_duty * limited_max_pwm_duty / MAX_PWM_DUTY; // adjust_duty * 旋钮限制的占空比系数

        // if (limited_adjust_pwm_duty >= 5950) // 大于该值，直接输出最大功率，防止从MIN扭到MAX时，输出不了最大功率
        // {
        //     limited_adjust_pwm_duty = adjust_duty;
        // }

        // 如果 limited_max_pwm_duty 改变，这里要更新 adjust_pwm_channel_ x _duty 的状态
        // if (last_limited_max_pwm_duty != limited_max_pwm_duty)
        // {
        //     adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(adjust_pwm_channel_0_duty);
        //     adjust_pwm_channel_1_duty = get_pwm_channel_x_adjust_duty(adjust_pwm_channel_1_duty);
        //     last_limited_max_pwm_duty = limited_max_pwm_duty;
        // }

        // if (0 == limited_adjust_pwm_duty)
        if (0 == limited_max_pwm_duty)
        {
            flag_is_last_limited_equal_zero = 1;
        }
    }
    else // 如果上一次限制的最大占空比是0，则要等旋钮扭到 xx% 的位置再使能旋钮的功能
    {
        if (adc_val >= KNOB_DIMMING_ADC_VAL_20_PERCENT) // 20%再开灯
        {
            flag_is_last_limited_equal_zero = 0;
        }
    }

#if USE_MY_DEBUG

    // printf("cur_level %u\n", knob_dimming_cur_level);

#endif // #if USE_MY_DEBUG

#endif
}
