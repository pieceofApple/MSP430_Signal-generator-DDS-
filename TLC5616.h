#if !defined(TLC5616_H)
#define TLC5616_H

#include <msp430.h>
//FS     片选端口P4.4
//CLK    时钟端口P4.3
//DIN    数据输入P4.1
//CS	 		 GND
//硬件SPI
void init_SPI()
{
    //STE
    P2SEL &= ~BIT7;
    P2DIR |= BIT7;
    P2OUT |= BIT7;
//    P2SEL &=~BIT3;
//    P2DIR |=BIT3;
//    P2OUT |=BIT3;
    // 配置UCB0模块的端口
    P4SEL |= BIT1 +BIT2+BIT3;

    // 配置UCB1模块
    UCB1CTL1 |= UCSWRST+UCSSEL_2; // 进入SPI软件复位状态//data rate
    UCB1CTL0 |= UCMSB | UCMST | UCSYNC; // 配置SPI模块的控制寄存器，设置为3线模式，主模式，时钟相位为第二个边沿采样，MSB优先，同步模式
    UCB1CTL0 &=~ UCCKPH+UCCKPL;
    UCB1BR0 |= 0x02; // 设置SPI时钟速度为SMCLK/2
    UCB1BR1 = 0;
    UCB1CTL1 &= ~UCSWRST; // 退出SPI软件复位状态
}
void init_ref_voltage() {
    P5SEL |= BIT0;
    // 配置REF模块为使用外部参考电压，参考电压为2.5V
    REFCTL0 &= ~REFMSTR; // 允许单个参考源的使用
    REFCTL0 |= REFVSEL_2 | REFOUT; // 选择2.5V参考电压并使能REFOUT输出
    ADC12CTL0 |= ADC12REFON+ADC12REF2_5V; //
    ADC12CTL2 |= ADC12REFOUT;
}
void TCL5616_IO_init()
{
    P4DIR |= BIT3+BIT1;
    P4OUT &=~BIT1+BIT3;
    P4DIR |=BIT4;
	P4OUT |=BIT4;
}
//硬件SPI
void write_TLC5616(unsigned int data)
{
    UCB1TXBUF = data >> 8; // 将高8位数据写入TX缓冲区
    while (!(UCB1IFG & UCTXIFG)); // 等待数据发送完成

    UCB1TXBUF = data & 0xFF; // 将低8位数据写入TX缓冲区
    while (!(UCB1IFG & UCTXIFG)); // 等待数据发送完成
}
//软件SPI
void DA_OUTPUT(unsigned int value)
{
	unsigned char i;  //定义变量用于循环
	//value<<=4;        //int类型为16位，仅需12位则去掉高4位方便取出
    P4OUT |=  BIT3;    //时钟CLK=1;     
	P4OUT &=~ BIT4;    //FS=0;

	/*循环16次取出16位写入模块寄存器*/
	for(i = 0; i < 16; i++)
	{
		if(value & 0X8000) P4OUT |= BIT1;
		else P4OUT &=~ BIT1;
        P4OUT &=~ BIT3;//CLK=1;
		value<<=1;
		P4OUT |= BIT3;//CLK=0;
	}
	P4OUT |= BIT3;//CLK=1;             //恢复默认低电平状态
	P4OUT |= BIT4;//FS=1;
}
#endif // TLC5616_H
