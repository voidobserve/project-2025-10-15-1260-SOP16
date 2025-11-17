#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "include.h"
#include "my_config.h"

/*
    检测发动机时，adc使用的参考电压，单位：mV
*/
#define ADC_REF_VAL_ENGINE ((u16)4200)

// 170VAC附近的ad值死区
#define ADC_DEAD_ZONE_NEAR_170VAC (u16)((u32)31 * 4096 / ADC_REF_VAL_ENGINE)

// 电压有波动时，最大ad值和最小ad值之前的差值
#define ADC_OVER_DRIVE_VAL (u16)((u32)83 * 4096 / ADC_REF_VAL_ENGINE)

/*
    170V AC 对应的检测脚ad值 ==
    检测脚电压值（单位：mV） * 4096(对应12位精度) / adc参考电压(单位：mV)
*/
#define ADC_VAL_170VAC (u16)((u32)1687 * 4096 / ADC_REF_VAL_ENGINE)

/*
    小于 170V AC时，对应的检测脚ad值 ==
    检测脚电压值（单位：mV） * 4096(对应12位精度) / adc参考电压(单位：mV)
*/
#define ADC_VAL_LOWER_THAN_170VAC (u16)((u32)1513 * 4096 / ADC_REF_VAL_ENGINE)

extern volatile bit flag_is_time_to_check_engine;

void according_pin9_to_adjust_pwm(void);

#endif
