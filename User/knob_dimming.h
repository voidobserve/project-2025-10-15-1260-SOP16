#ifndef __KNOB_DIMMING_H
#define __KNOB_DIMMING_H
// 旋钮调光头文件

#include "my_config.h"

#define KNOB_DIMMING_LEVELS (160) // 旋钮调光的分级
// #define KNOB_DIMMING_ADC_DEAD_ZONE (200)                             // 最大电压值和最小电压值使用到的死区 (客户反馈扭到10%占空比就关了)
// #define KNOB_DIMMING_ADC_DEAD_ZONE (50)                             // 最大电压值和最小电压值使用到的死区
// #define KNOB_DIMMING_ADC_DEAD_ZONE (25) // 最大电压值和最小电压值使用到的死区
// #define KNOB_DIMMING_ADC_DEAD_ZONE (16) // 最大电压值和最小电压值使用到的死区【实际测试还是在旋钮10%的时候就关灯了】
#define KNOB_DIMMING_ADC_DEAD_ZONE (11) // 最大电压值和最小电压值使用到的死区
// #define KNOB_DIMMING_ADC_DEAD_ZONE (7)                              // 最大电压值和最小电压值使用到的死区【这里死区太小，灯光会无法关闭】
// #define KNOB_DIMMING_MAX_ADC_VAL (4095 - KNOB_DIMMING_ADC_DEAD_ZONE) // 客户在引脚上测得最大的电压值是4.87V，就是VCC的电压，这里最好再加上死区
// #define KNOB_DIMMING_MAX_ADC_VAL (4006 - KNOB_DIMMING_ADC_DEAD_ZONE) //
// #define KNOB_DIMMING_MAX_ADC_VAL (4001 - KNOB_DIMMING_ADC_DEAD_ZONE) // 最大功率458W，还没到461W
#define KNOB_DIMMING_MAX_ADC_VAL (3996 - KNOB_DIMMING_ADC_DEAD_ZONE) //
#define KNOB_DIMMING_MIN_ADC_VAL (479 + KNOB_DIMMING_ADC_DEAD_ZONE)  // 客户引脚上测得最小电压值是0.57V

// 旋钮调光其中一级对应的ad值
#define KNOB_DIMMING_ADC_VAL_PER_LEVEL \
    ((u16)(KNOB_DIMMING_MAX_ADC_VAL - KNOB_DIMMING_MIN_ADC_VAL) / KNOB_DIMMING_LEVELS) // 若干分级中，其中的一级对应的ad值

/*
    旋钮调光的每一级分级之间的死区
    4096 - 479 = 3617
    分成160级，3617 / 160 = 22.60625（取整为 22）
    在 22 取 10% -> 2.2（取整为2）作为死区
    在 22 取 20% -> 4.4（取整为4）作为死区
    #define KNOB_DIMMING_ADC_DEAD_ZONE_PER_LEVEL \
    ((u8)(KNOB_DIMMING_ADC_VAL_PER_LEVEL * 2 / 10))
*/

// 定义旋钮各个挡位下，单片机检测到的ad值，实际会因为旋钮的机械结构而有偏差，比如从min扭到max，从max扭到min，检测到的值会不一样
#define KNOB_DIMMING_ADC_VAL_20_PERCENT 1245
#define KNOB_DIMMING_ADC_VAL_40_PERCENT 2286
#define KNOB_DIMMING_ADC_VAL_50_PERCENT 2834
#define KNOB_DIMMING_ADC_VAL_60_PERCENT 3260
// #define KNOB_DIMMING_ADC_VAL_80_PERCENT

extern volatile u16 limited_max_pwm_duty;    // 存放限制的最大占空比
// extern volatile u16 limited_adjust_pwm_duty; // 存放旋钮限制之后的，待调整的占空比值

// 根据旋钮，限制当前的最大占空比
void update_max_pwm_duty_coefficient(void);

#endif
