#ifndef _ADC_H
#define _ADC_H

#include <stdio.h>
#include <include.h>
#include "pwm.h"
#include <string.h>

// ADC 参考电压 VCCA，单位：mV ，客户测得是4.87V
#define ADC_REF_VOLTAGE_VAL ((u16)4870) // 单位：mV
// #define ADC_REF_VOLTAGE_VAL ((u16)4200) // 只在测试时使用

// 定义检测adc的通道:
enum
{
    ADC_SEL_PIN_NONE = 0,
    ADC_SEL_PIN_GET_TEMP = 0x01, // 根据热敏电阻一端来配置ADC
    ADC_SEL_PIN_GET_VOL = 0x02,  // 根据9脚来配置ADC
    ADC_SEL_PIN_P31 = 0x03,      // P31，7脚
    ADC_SEL_PIN_FAN_DETECT,      // P13， 芯片的1脚，检测风扇是否异常的引脚

};

enum
{
    ADC2_STATUS_NONE = 0,

    ADC2_STATUS_SEL_GET_TEMP_WAITING, // 等待adc稳定
    ADC2_STATUS_SEL_GET_TEMP,         // 切换至检测热敏电阻的通道

    ADC2_STATUS_SEL_FAN_DETECT_WAITING, // 等待adc稳定
    ADC2_STATUS_SEL_FAN_DETECT,         // 切换至检测风扇的通道
};

extern volatile u8 cur_adc2_status; // 状态机，表示当前adc2的状态

// 存放温度状态的变量
extern volatile u8 temp_status;

// 标志位，由定时器扫描并累计时间，表示当前风扇是否异常
extern volatile bit flag_tim_scan_fan_is_err;
extern volatile u8 cur_fan_status; // 当前风扇状态

extern volatile u16 adc_val_from_engine; // 存放 从发动机一侧 检测到的ad值
extern volatile u16 adc_val_from_knob;   // 存放 从旋钮一侧 采集到的ad值
extern volatile u16 adc_val_from_temp; // 存放 从热敏电阻一侧 采集到的ad值
extern volatile u16 adc_val_from_fan; // 存放 检测风扇一侧 采集到的ad值

// adc单次采集+转换（没有滤波）
// u16 adc_get_val_single(void);

// 获取一次adc采集+滤波后的值
// u16 adc_get_val(void);

void adc_pin_config(void); // adc相关的引脚配置，调用完成后，还未能使用adc 

u32 get_voltage_from_pin(void); // 从引脚上采集滤波后的电压值

// void adc_update_pin_9_adc_val(void);

// void adc_scan(void);
void temperature_scan(void);
void set_duty(void);

void fan_scan(void);

void adc_config(void);
void adc0_channel_sel(void);
void adc1_channel_sel(void);
void adc2_channel_sel(u8 adc_sel_pin);

#endif