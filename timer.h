#include <msp430.h>

void init_timer(void)
{
    TA0CCTL0 &=~ CCIE; // 等待需要开启时再开启
    TA0CTL = TASSEL_2 + MC_1 + ID_0; // 选择 SMCLK 作为时钟源，计数模式为增计数，分频系数为 8
    TA0CCR0 = 400 - 1; // 计数器计数到 32767，即 1 秒
}

