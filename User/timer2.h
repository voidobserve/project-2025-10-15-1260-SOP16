#ifndef __TIMER2_H
#define __TIMER2_H

#include "my_config.h"

extern volatile bit flag_is_pwm_add_time_comes; // 标志位，pwm占空比递增时间到来
extern volatile bit flag_is_pwm_sub_time_comes; // 标志位，pwm占空比递减时间到来
  

void timer2_config(void);

#endif