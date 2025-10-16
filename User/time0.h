#ifndef _TIME0_H
#define _TIME0_H

#include "include.h"

#include <stdio.h>
#include "adc.h"

// extern volatile bit tmr0_flag;
extern volatile bit flag_time_comes_during_power_on; // 标志位，开机缓启动期间，调节时间到来

void tmr0_config(void);
// void tmr0_enable(void);  // 开启定时器，开始计时
// void tmr0_disable(void); // 关闭定时器，清空计数值

#endif
