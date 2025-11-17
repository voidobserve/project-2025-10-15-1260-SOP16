注意
---
- 编译器配置：`C51` -> `Code Optimization` -> `Level`必须要选择`8:Reuse Common Entry Code`，`Code Optimization` -> `Emphasis`必须要选择`Favor Speed`

---

- 如果同时有线控调光和遥控调光的功能，以线控调光的挡位作为当前最大的占空比，遥控调节在这个基础上进一步调节

参考客户原话：
如果有线控调光，同时使用遥控调光，就按线控调光所在的位置来确定遥控调光的大小。客户几乎只会二选一，因为每增加一种调光，都要加钱。

--- 

定时器会根据`adjust_pwm_channel_x_duty`的值来调节`cur_pwm_channel_x_duty`
最终将`cur_pwm_channel_x_duty`的值写入对应的pwm寄存器中

要想修改pwm占空比的值，应该先修改 expect_adjust_pwm_channel_x_duty ，
再调用 get_pwm_channel_x_adjust_duty() 更新 adjust_pwm_channel_x_duty
最后让定时器自行调节，实现缓慢变化的效果

---

- 脚位和功能

1 -- P13 检测风扇工作状态是否异常
2 -- P12 控制风扇开关
3 -- P11 RF-433功能使能脚 
4 -- P00 DEBUG 打印引脚
5 -- P02 NC
6 -- P03 RF-433 信号接收脚
7 -- P31 检测旋钮调光
8 -- P30 检测热敏电阻一侧的电压 温度检测引脚

9 -- P27 检测发动机是否稳定
10 -- P21 NC
11 -- P17 NC
12 -- VSS
13 -- VCC
14 -- P16 一路PWM输出，控制一路灯光
15 -- P15 一路PWM输出，控制一路灯光
16 -- P14 目前固定输出低电平

demo板上面的引脚

P01  可以用于调试
P05  可以用于调试
P06  可以用于调试