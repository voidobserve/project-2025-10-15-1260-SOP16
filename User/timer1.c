#include "timer1.h"

#define TMR1_CNT_TIME 15200 // 计数周期，15200 * 0.65625us 约等于10000us--10ms

volatile u32 tmr1_cnt = 0; // 定时器TMR0的计数值（每次在中断服务函数中会加一，用于累计温度过热的时间）

/**
 * @brief 配置定时器TMR0，定时器默认关闭
 */
void tmr1_config(void)
{
    __SetIRQnIP(TMR1_IRQn, TMR1_IQn_CFG); // 设置中断优先级（TMR1）

    TMR1_CONL &= ~(TMR_PRESCALE_SEL(0x07)); // 清除TMR1的预分频配置寄存器
    // 配置TMR1的预分频，为32分频，即21MHz / 32 = 0.65625MHz，约0.67us计数一次
    // （实际测试和计算得出这个系统时钟是21MHz的，但是还是有些误差，不是准确的21MHz）
    TMR1_CONL |= TMR_PRESCALE_SEL(0x05);
    TMR1_CONL &= ~(TMR_MODE_SEL(0x03)); // 清除TMR1的模式配置寄存器
    TMR1_CONL |= TMR_MODE_SEL(0x01);    // 配置TMR1的模式为计数器模式，最后对系统时钟的脉冲进行计数

    TMR1_CONH |= TMR_PRD_PND(0x01); // 清除TMR1的计数标志位，表示未完成计数
    TMR1_CONH |= TMR_PRD_IRQ_EN(1); // 使能TMR1的计数中断

    // 配置TMR1的计数周期
    TMR1_PRH = TMR_PERIOD_VAL_H((TMR1_CNT_TIME >> 8) & 0xFF);
    TMR1_PRL = TMR_PERIOD_VAL_L((TMR1_CNT_TIME >> 0) & 0xFF);

    // 清除TMR1的计数值
    TMR1_CNTL = 0;
    TMR1_CNTH = 0;

    TMR1_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除TMR1的时钟源配置寄存器
    // TMR1_CONL |= TMR_SOURCE_SEL(0x07); // 配置TMR1的时钟源，使用系统时钟
    TMR1_CONL |= TMR_SOURCE_SEL(0x05); // 配置TMR1的时钟源，不用任何时钟

    // __EnableIRQ(TMR1_IRQn);			   // 使能中断
    __DisableIRQ(TMR1_IRQn); // 禁用中断
    IE_EA = 1;               // 打开总中断
}

/**
 * @brief 开启定时器TMR1，开始计时
 */
void tmr1_enable(void)
{
    // 重新给TMR1配置时钟
    TMR1_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
    TMR1_CONL |= TMR_SOURCE_SEL(0x06);    // 配置定时器的时钟源，使用系统时钟（约21MHz）

    __EnableIRQ(TMR1_IRQn); // 使能中断
    IE_EA = 1;              // 打开总中断
}

/**
 * @brief 关闭定时器1，清空计数值
 */
void tmr1_disable(void)
{
    // 不给定时器提供时钟，让它停止计数
    TMR1_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
    TMR1_CONL |= TMR_SOURCE_SEL(0x05);    // 配置定时器的时钟源，不用任何时钟

    // 清除定时器的计数值
    TMR1_CNTL = 0;
    TMR1_CNTH = 0;

    __DisableIRQ(TMR1_IRQn); // 关闭中断（不使能中断）
}

// 定时器TMR1中断服务函数
void TIMR1_IRQHandler(void) interrupt TMR1_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR1_IRQn);

    // ---------------- 用户函数处理 -------------------

    // 周期中断
    if (TMR1_CONH & TMR_PRD_PND(0x1))
    {
        TMR1_CONH |= TMR_PRD_PND(0x1); // 清除pending

        tmr1_cnt++; // 检测温度时使用到
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR1_IRQn);
}
