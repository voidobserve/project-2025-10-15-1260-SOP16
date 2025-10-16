#ifndef _ADC_H
#define _ADC_H

#include <stdio.h>
#include <include.h>
#include "pwm.h"
#include <string.h>

// 定义检测adc的通道:
enum
{
    ADC_SEL_PIN_NONE = 0,
    ADC_SEL_PIN_GET_TEMP = 0x01, // 根据热敏电阻一端来配置ADC
    ADC_SEL_PIN_GET_VOL = 0x02,  // 根据9脚来配置ADC
    ADC_SEL_PIN_P31 = 0x03,      // P31，7脚
    ADC_SEL_PIN_FAN_DETECT,      // P13， 芯片的1脚，检测风扇是否异常的引脚

};

// 存放温度状态的变量
extern volatile u8 temp_status;

// 标志位，由定时器扫描并累计时间，表示当前风扇是否异常
extern volatile bit flag_tim_scan_fan_is_err;
extern volatile u8 cur_fan_status; // 当前风扇状态

extern volatile u16 adc_val_pin_9; // 存放9脚采集到的ad值

// adc单次采集+转换（没有滤波）
u16 adc_get_val_single(void);

// 获取一次adc采集+滤波后的值
u16 adc_get_val(void);

void adc_pin_config(void);      // adc相关的引脚配置，调用完成后，还未能使用adc
void adc_sel_pin(const u8 pin); // 切换adc采集的引脚，并配置好adc
// void adc_single_getval(void);   // adc完成一次转换

u32 get_voltage_from_pin(void); // 从引脚上采集滤波后的电压值

void adc_update_pin_9_adc_val(void);

// void adc_scan(void);
void temperature_scan(void);
void set_duty(void);

void fan_scan(void);

#endif