#include <msp430.h>
#include <string.h>
#include "lcd1602.h"
#define CPU_F ((double)8000000)
#define delay_us(x) __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))
#define Busy        0x80
#define CLR_RS P8OUT&=~BIT1;    //RS = P1.3
#define SET_RS P8OUT|=BIT1;
#define CLR_EN P8OUT&=~BIT2;    //EN = P1.5
#define SET_EN P8OUT|=BIT2;

//74HC164驱动函数，P3.7-CLK，P2.3-data
void HC164_Data(unsigned char sum) {
    int k=0;
    for (k = 0; k < 8; k++) {
        P3OUT &= ~BIT7;//164-CLK
        if (sum & 0x80)
            P2OUT |= BIT3;
        else
            P2OUT &= ~BIT3;
        P3OUT |= BIT7;
        sum <<= 1;
    }
}

void Disp1int(unsigned char data)
{
    LCD1602_WriteDATA(data);
}
void DispStr(unsigned char *ptr)
{
    unsigned int i,n;
    n=strlen(ptr);
    for (i=0;i<n;i++)
    {
        Disp1int(ptr[i]);
    }
}

void LCD1602_WriteCMD(unsigned char cmd)
{
        CLR_EN;
        CLR_RS;     //指令
        HC164_Data(cmd);       //指令数据传到P0口待发送给LCD1602
        delay_ms(1);
        SET_EN;     //LCD1602使能口
        delay_ms(10);
        CLR_EN;     //产生下降沿
        delay_ms(10);
}
void LCD1602_WriteDATA(unsigned char Data)
{
        HC164_Data(Data);
        SET_RS;
        SET_EN;
        CLR_EN;
        delay_ms(10);
}
void LCD1602_Writenumber(unsigned int num) {
    if (num < 0) {
        LCD1602_WriteDATA('-');
        num = -num;
    }
    if (num == 0) {
        LCD1602_WriteDATA('0');
        return;
    }
    int digits[10];
    int i = 0;
    while (num > 0) {
        digits[i++] = num % 10;
        num /= 10;
    }
    int j=0;
    for (j = i - 1; j >= 0; j--) {
        LCD1602_WriteDATA(digits[j] + '0');
    }
}
void LCD1602_Initi()
    {
        P3DIR |= BIT7;//74HC164的clk
        P8DIR |= BIT1 + BIT2;//P8.1----RS,P8.2---EN
        P2DIR |= BIT3 + BIT6;//P2.3---164input;P2.6---VL
        LCD1602_WriteCMD(0x38); //  置功能，8位总线，双行显示，显示5X7的点阵字符
        LCD1602_WriteCMD(0x0f); //  显示开关控制,开显示, 有光标,闪烁
        LCD1602_WriteCMD(0x06); //  光标和显示位置设置,  光标移动方向右移,屏幕上所有文字不动
        LCD1602_WriteCMD(0x01); //  清显示，指令码01H，光标复位到地址00H位置
        delay_ms(10);
    }
//VL脚的PWM波控制
void PWM_lcd(void) {
    //PWM
    P3DIR |= BIT5 + BIT6;  //TB0.5,输出PWM波信号
    P3SEL |= BIT5 + BIT6;
    //P3OUT |= BIT6;
    TB0CTL |= TBSSEL_1 + MC_1;//定时器TB0，时钟源选择ACLK，向上计数。
    TB0CCTL0 |= CAP;//第0个通道为 比较模式

    TB0CCTL5 |= CAP;//第5个通道为 比较模式，其实本来默认就是比较模式，pwm捕捉做定时器
    TB0CCTL5 = OUTMOD_7;//第五个通道设置为输出模式，产生PWM波的信号

    TB0CCTL6 |= CAP;//第6个通道为 比较模式，其实本来默认就是比较模式，pwm捕捉做定时器
    TB0CCTL6 = OUTMOD_7;//第六个通道设置为输出模式，产生PWM波的信号

    TB0CCR0 = 800;
    TB0CCR5 = 350;//未调前默认显示1800
    TB0CCR6 = 900;//未调前默认显示2000
}
