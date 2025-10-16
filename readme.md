
如果同时有线控调光和遥控调光的功能，以线控调光的挡位作为当前最大的占空比，遥控调节在这个基础上进一步调节

参考客户原话：
如果有线控调光，同时使用遥控调光，就按线控调光所在的位置来确定遥控调光的大小。客户几乎只会二选一，因为每增加一种调光，都要加钱。


定时器会根据 adjust_pwm_channel_x_duty 的值来调节 cur_pwm_channel_x_duty
最终将 cur_pwm_channel_x_duty 的值写入对应的pwm寄存器中

要想修改pwm占空比的值，应该先修改 expect_adjust_pwm_channel_x_duty ，
再调用 get_pwm_channel_x_adjust_duty() 更新 adjust_pwm_channel_x_duty
最后让定时器自行调节，实现缓慢变化的效果



