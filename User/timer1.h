#ifndef __TIMER1_H
#define __TIMER1_H

// #include "my_config.h"
#include "include.h"

extern volatile u32 tmr1_cnt; // 定时器TMR1的计数值（每次在中断服务函数中会加一，用于累计温度过热的时间）

void tmr1_config(void);  // 配置定时器，定时器默认关闭
void tmr1_enable(void);  // 开启定时器，开始计时
void tmr1_disable(void); // 关闭定时器，清空计数值

#endif
