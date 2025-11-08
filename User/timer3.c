#include "my_config.h"
#include "timer3.h"

/*
    将 timer3 配置为每 935us 产生一次中断的定时器
    935us == 0.000935s，转换成频率：
    1 / 0.00935 == 1069.518716577540106951871657754 Hz
*/

// 实际测试在935us，如果有其他定时器中断，时间会有误差，在910~960us
#define TIMER3_PERIOD_VAL (SYSCLK / 128 / 1069 - 1) // 周期值=系统时钟/分频/频率 - 1

void timer3_config(void)
{
    __EnableIRQ(TMR3_IRQn); // 使能timer3中断
    IE_EA = 1;              // 使能总中断

    // 设置timer3的计数功能，配置一个频率为 kHz的中断
    TMR_ALLCON = TMR3_CNT_CLR(0x1);                               // 清除计数值
    TMR3_PRH = TMR_PERIOD_VAL_H((TIMER3_PERIOD_VAL >> 8) & 0xFF); // 周期值
    TMR3_PRL = TMR_PERIOD_VAL_L((TIMER3_PERIOD_VAL >> 0) & 0xFF);
    TMR3_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1);                          // 计数等于周期时允许发生中断
    TMR3_CONL = TMR_SOURCE_SEL(0x7) | TMR_PRESCALE_SEL(0x7) | TMR_MODE_SEL(0x1); // 选择系统时钟，128分频，计数模式
}

void TIMR3_IRQHandler(void) interrupt TMR3_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR3_IRQn);

    // ---------------- 用户函数处理 -------------------
    if (0 == (TMR3_CONH & TMR_PRD_PND(0x1)))
    {
        // 不是该中断触发（运算之后是非0值，才是该中断触发），直接退出
        // 退出中断设置IP，不可删除
        __IRQnIPnPop(TMR3_IRQn);
        return;
    }

    TMR3_CONH |= TMR_PRD_PND(0x1); // 清除中断标志

    //
    if (0 == (ADC_CFG0 & 0x01))
    {
        ADC_CFG0 |= 0x01 << 0; // 开启adc0转换
    }

    if (0 == ((ADC_CFG0 >> 1) & 0x01))
    {
        ADC_CFG0 |= 0x01 << 1; // 开启 adc1 转换
    }

    {
        static u8 cnt;
        cnt++;
        if (cnt >= 10)
        {
            cnt = 0;
            flag_is_time_to_check_engine = 1;

            // 下面这段代码的中断频率不能比adc中断还要快
            // 每次执行到这里，不用等adc2稳定，因为中断时间间隔比adc2稳定的时间还要长
            if (ADC2_STATUS_NONE == cur_adc2_status ||
                (ADC2_STATUS_SEL_FAN_DETECT == cur_adc2_status))
            {
                // adc2切换至检测热敏电阻的通道，并等待adc2稳定
                adc2_channel_sel(ADC_SEL_PIN_GET_TEMP);
                cur_adc2_status = ADC2_STATUS_SEL_GET_TEMP_WAITING;
            }
            else if (ADC2_STATUS_SEL_GET_TEMP_WAITING == cur_adc2_status)
            {
                ADC_CFG0 |= 0x01 << 2;                      // 开启 adc2 转换
                cur_adc2_status = ADC2_STATUS_SEL_GET_TEMP; // 表示当前已经切换到了 检测热敏电阻的通道
            }
            else if (ADC2_STATUS_SEL_GET_TEMP == cur_adc2_status)
            {
                // adc2切换至检测风扇的通道，并等待adc2稳定
                adc2_channel_sel(ADC_SEL_PIN_FAN_DETECT);
                cur_adc2_status = ADC2_STATUS_SEL_FAN_DETECT_WAITING;
            }
            else if (ADC2_STATUS_SEL_FAN_DETECT_WAITING == cur_adc2_status)
            {
                ADC_CFG0 |= 0x01 << 2;                        // 开启 adc2 转换
                cur_adc2_status = ADC2_STATUS_SEL_FAN_DETECT; // 检测风扇的通道
            }
        }
    }

    __IRQnIPnPop(TMR3_IRQn);
}