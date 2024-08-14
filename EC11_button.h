#if !defined(EC11_BUTTON_H)
#define EC11_BUTTON_H

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "TLC5616.h"
#include "lcd1602.h"
//波形幅值，频率标志位
unsigned char flag_count=0,flag_menu=1;
/*
正弦波（Sine Wave）：sin  锯齿波（Sawtooth Wave）：saw 三角波（Triangle Wave）：tri
方波（Square Wave）：sq 或 sqr 幅值（Amplitude）：A 频率（Frequency）：f
*/
//pin_A_status = (P2IN & BIT1) ? 1 : 0;//A相，该IO口状态
//pin_B_status = (P2IN & BIT0) ? 1 : 0;
/*************背光调节*************/
int Backlight_duty_cycle=10000,contrast_duty_cycle=2000;//PWM波周期，占空比
static int cycle=20000;//周期
//IO口定义 A,B相
#define EC11_PIN_A BIT5
#define EC11_PIN_B BIT4
//EC11正反旋转档位
int forward_count=0,reverse_count=0,count;
//幅值，频率
unsigned char amplitude=0,frequence=0;
int amplitude_count=0,frequence_count=1;
//EC11按键按下，即选中当前光标的选项
unsigned char flag_select=0;
//光标
unsigned char cursor_row = 0; // 光标所在行
unsigned char cursor_col = 0; // 光标所在列
//幅值，频率是否被选中，选中则可以通过EC11旋转改变
unsigned char flag_amplitude=0,flag_frequence=0;
/******EC11*********
 ******************/
//IO口初始化
void IO_Button_init(void)
{
    //EC11 B,A相位，S1（按键）
    P2DIR &= ~(EC11_PIN_A + EC11_PIN_B+BIT0); // 将旋转编码器EC11的引脚设置为输入
    P2REN |= EC11_PIN_A + EC11_PIN_B+BIT0; // 启用旋转编码器EC11的上拉电阻
    P2OUT |= EC11_PIN_A + EC11_PIN_B+BIT0; // 将旋转编码器EC11的引脚设置为上拉输入
    
    P2IES |= (EC11_PIN_A + EC11_PIN_B+BIT0); // 将旋转编码器EC11的引脚的中断触发方式设置为下降沿
    P2IFG &= ~(EC11_PIN_A + EC11_PIN_B+BIT0); // 清除旋转编码器EC11的中断标志位
    P2IE |= EC11_PIN_A + EC11_PIN_B+BIT0; // 启用旋转编码器EC11的中断

    //上下左右四个按键P1.0,P1.1,P1.6,P1.7
    P1DIR &= ~(BIT2 + BIT3 + BIT4 + BIT5); // 将P1.0、P1.1、P1.6和P1.7引脚配置为输入模式
    P1REN |= BIT2 + BIT3 + BIT4 + BIT5;   // 使能P1.0、P1.1、P1.6和P1.7引脚的上拉电阻
    P1OUT |= BIT2 + BIT3 + BIT4 + BIT5;   // 将P1.0、P1.1、P1.6和P1.7引脚的上拉电阻设置为上拉

    P1IES |= BIT2 + BIT3 + BIT4 + BIT5;   // 配置下降沿触发中断
    P1IE |= BIT2 + BIT3 + BIT4 + BIT5;   //中断使能
    P1IFG &= ~(BIT2 + BIT3 + BIT4 + BIT5); // 清除中断标志

    //P1IFG3，P1IFG5初始为1；关闭
    //P1IFG &=~ BIT3+BIT5;
    __enable_interrupt();
}

/*
//位置分布
    0   1   2
0  BL  CTR  SIN
1 TRI  SQR  SAW
*/
//判断EC11正反转，并调节背光，对比度（中断内调节较快，中断外调节反应迟钝）
void EC11_direction()
{
        if((P2IN & BIT4) ? 1 : 0 != (P2IN & BIT5) ? 1 : 0)
    {
        if((P2IN & BIT4) ? 1 : 0==1)
        {
            if(cursor_row==0 && cursor_col==0 && flag_select==1){//BL
            //对比度，背光调节
                Backlight_duty_cycle+=1000; 
                if(Backlight_duty_cycle>20000)
                    Backlight_duty_cycle=20000;
                TB0CCR6 = Backlight_duty_cycle;
            }
            if(cursor_row==0 && cursor_col==1 && flag_select==1){//CTR
                contrast_duty_cycle+=500;
                if(contrast_duty_cycle>10000)
                    contrast_duty_cycle=10000;
                TB0CCR5 = contrast_duty_cycle;
            }
            //波形调幅，调频
            if(flag_amplitude){
                amplitude_count++;
            }
            if(flag_frequence) frequence_count++;
        }
        else if((P2IN & BIT5) ? 1 : 0==0){
            if(cursor_row==0 && cursor_col==0 && flag_select==1){//BL
            //对比度，背光调节
                Backlight_duty_cycle-=1000; 
                if(Backlight_duty_cycle<0)
                    Backlight_duty_cycle=0;
                TB0CCR6 = Backlight_duty_cycle;
            }
            if(cursor_row==0 && cursor_col==1 && flag_select==1){//CTR
                contrast_duty_cycle-=500;
                if(contrast_duty_cycle<0)
                    contrast_duty_cycle=0;
                TB0CCR5 = contrast_duty_cycle;
            }
            //波形调幅，调频
            if(flag_amplitude){
                amplitude_count--;
            }
            if(flag_frequence) frequence_count--;
        }
        flag_count=1;
        while((!((P2IN & BIT4) ? 1 : 0))||(!((P2IN & BIT5) ? 1 : 0)));//AB相电平状况,不同时循环空转
    }
}
/************************************/


/*****LCD1602*******
 ******************/
//按键控制部分
// 保存P1IFG寄存器的值
uint8_t p1ifg=0;
void switch_cursor_interrupt()
{ 
    if(flag_select)
    {
        if (p1ifg & BIT4) // P1.4引脚触发中断,左按键
        {
            LCD1602_WriteCMD(0XC0+2);
            flag_amplitude=1;
            flag_frequence=0;
        }
        if (p1ifg & BIT5) // P1.5引脚触发中断，右按键
        {
            LCD1602_WriteCMD(0XC0+8);
            flag_frequence=1;
            flag_amplitude=0;
        }
    }
    else if(flag_select==0)
    {
        if (p1ifg & BIT2) // P1.2引脚触发中断
        {
            cursor_row=0;
        }
        if (p1ifg & BIT3) // P1.3引脚触发中断
        {
            cursor_row=1;
        }
        if (p1ifg & BIT4) // P1.4引脚触发中断,左按键
        {
            cursor_col--;//0,1,2
            if(cursor_col<1) cursor_col=0;
        }
        if (p1ifg & BIT5) // P1.5引脚触发中断，右按键
        {
            cursor_col++;
            if(cursor_col>1) cursor_col=2;
        }
    }
    p1ifg = 0;
}

void switch_key_while()
{
    LCD1602_WriteCMD(0x0E);
    if(cursor_row==0 && cursor_col==0) {
        LCD1602_WriteCMD(0X80+1);
        if(flag_select && flag_count){
            LCD1602_WriteCMD(0X01);
            LCD1602_WriteCMD(0X80);
            DispStr("BL");
            LCD1602_WriteCMD(0XC0);
            LCD1602_Writenumber(Backlight_duty_cycle);
            flag_count=0;
        }
    }//BL
    if(cursor_row==0 && cursor_col==1) {
        LCD1602_WriteCMD(0X80+7);
        if(flag_select && flag_count){
            LCD1602_WriteCMD(0X01);
            LCD1602_WriteCMD(0X80);
            DispStr("CTR");
            LCD1602_WriteCMD(0XC0);
            LCD1602_Writenumber(contrast_duty_cycle);
            flag_count=0;
        }
    }//CTR
    if(cursor_row==0 && cursor_col==2) {
        LCD1602_WriteCMD(0X80+13);
        if(flag_select && flag_count){
            //存入正弦波表：
            LCD1602_WriteCMD(0X01);
            LCD1602_WriteCMD(0X80);
            DispStr("SIN");
            LCD1602_WriteCMD(0XC0);
            DispStr("AMP:");
            LCD1602_Writenumber(amplitude_count);
            DispStr("FRE:");
            LCD1602_Writenumber(frequence_count);
            flag_count=0;
        }
    }//SIN
    if(cursor_row==1 && cursor_col==0) {
        LCD1602_WriteCMD(0XC0+1);
        if(flag_select && flag_count){
            LCD1602_WriteCMD(0X01);
            LCD1602_WriteCMD(0X80);
            DispStr("TRI");
            LCD1602_WriteCMD(0XC0);
            DispStr("AMP:");
            LCD1602_Writenumber(amplitude_count);
            DispStr("FRE:");
            LCD1602_Writenumber(frequence_count);
            flag_count=0;
        }
    }//TRI
    if(cursor_row==1 && cursor_col==1) {
        LCD1602_WriteCMD(0XC0+7);
        if(flag_select && flag_count){
            LCD1602_WriteCMD(0X01);
            LCD1602_WriteCMD(0X80);
            DispStr("SQR");
            LCD1602_WriteCMD(0XC0);
            DispStr("AMP:");
            LCD1602_Writenumber(amplitude_count);
            DispStr("FRE:");
            LCD1602_Writenumber(frequence_count);
            flag_count=0;
        }
    }//SQR
    if(cursor_row==1 && cursor_col==2) {
        LCD1602_WriteCMD(0XC0+13);
        if(flag_select && flag_count){
            LCD1602_WriteCMD(0X01);
            LCD1602_WriteCMD(0X80);
            DispStr("SAW");
            LCD1602_WriteCMD(0XC0);
            DispStr("AMP:");
            LCD1602_Writenumber(amplitude_count);
            DispStr("FRE:");
            LCD1602_Writenumber(frequence_count);
            flag_count=0;
        }
    }//SAW
    if(flag_menu){
            amplitude_count=0;
            frequence_count=0;
            LCD1602_WriteCMD(0x80);
            LCD1602_WriteCMD(0X01);
            DispStr("  BL  CTR  SIN ");
            LCD1602_WriteCMD(0XC0);
            DispStr(" TRI  SQR  SAW ");
            flag_menu=0;
        }
}
unsigned char flag_select_count;
void switch_EC11_button()//放EC11中断PORT2
{

    if(P2IFG & BIT0){
        TA0CCTL0 |=CCIE;
        flag_select=1;
        flag_count =1;
        flag_menu  =0;
        flag_select_count++;
        if(flag_select_count==5) {flag_select=0;flag_menu=1;TA0CCTL0 &=~ CCIE;flag_select_count=0;}//退出选中的波形
    }
}
//背光，对比度
// Backlight:背光 P2.4 用于BLACK——PWM输出调节背光
void Backlight_contrast_timer_init()
{
    //背光
	P3DIR |= BIT6; //LCD1602背光口K
	P3SEL |= BIT6;
	TB0CCR0 = cycle;
    TB0CCR6 = Backlight_duty_cycle; // 占空比
    TB0CCTL6 = OUTMOD_7; // 设置定时器A1的CCR2输出模式为PWM模式

    // contrast:对比度 Vo电压0~1.1V；p3.5
    P3DIR |= BIT5; //LCD1602背光口K
    P3SEL |= BIT5;
    TB0CCR0 = cycle;
    TB0CCR5 = contrast_duty_cycle; // 占空比
    TB0CCTL5 = OUTMOD_7; // 设置定时器A1的CCR2输出模式为PWM模式
    TB0CTL = TASSEL_2 + MC_1 + ID_0; // 设置定时器A1的时钟源为SMCLK，计数模式为Up，分频系数为1
}
/************************************/
#endif
