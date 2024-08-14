#if !defined(EC11_BUTTON_H)
#define EC11_BUTTON_H

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "TLC5616.h"
#include "lcd1602.h"
//���η�ֵ��Ƶ�ʱ�־λ
unsigned char flag_count=0,flag_menu=1;
/*
���Ҳ���Sine Wave����sin  ��ݲ���Sawtooth Wave����saw ���ǲ���Triangle Wave����tri
������Square Wave����sq �� sqr ��ֵ��Amplitude����A Ƶ�ʣ�Frequency����f
*/
//pin_A_status = (P2IN & BIT1) ? 1 : 0;//A�࣬��IO��״̬
//pin_B_status = (P2IN & BIT0) ? 1 : 0;
/*************�������*************/
int Backlight_duty_cycle=10000,contrast_duty_cycle=2000;//PWM�����ڣ�ռ�ձ�
static int cycle=20000;//����
//IO�ڶ��� A,B��
#define EC11_PIN_A BIT5
#define EC11_PIN_B BIT4
//EC11������ת��λ
int forward_count=0,reverse_count=0,count;
//��ֵ��Ƶ��
unsigned char amplitude=0,frequence=0;
int amplitude_count=0,frequence_count=1;
//EC11�������£���ѡ�е�ǰ����ѡ��
unsigned char flag_select=0;
//���
unsigned char cursor_row = 0; // ���������
unsigned char cursor_col = 0; // ���������
//��ֵ��Ƶ���Ƿ�ѡ�У�ѡ�������ͨ��EC11��ת�ı�
unsigned char flag_amplitude=0,flag_frequence=0;
/******EC11*********
 ******************/
//IO�ڳ�ʼ��
void IO_Button_init(void)
{
    //EC11 B,A��λ��S1��������
    P2DIR &= ~(EC11_PIN_A + EC11_PIN_B+BIT0); // ����ת������EC11����������Ϊ����
    P2REN |= EC11_PIN_A + EC11_PIN_B+BIT0; // ������ת������EC11����������
    P2OUT |= EC11_PIN_A + EC11_PIN_B+BIT0; // ����ת������EC11����������Ϊ��������
    
    P2IES |= (EC11_PIN_A + EC11_PIN_B+BIT0); // ����ת������EC11�����ŵ��жϴ�����ʽ����Ϊ�½���
    P2IFG &= ~(EC11_PIN_A + EC11_PIN_B+BIT0); // �����ת������EC11���жϱ�־λ
    P2IE |= EC11_PIN_A + EC11_PIN_B+BIT0; // ������ת������EC11���ж�

    //���������ĸ�����P1.0,P1.1,P1.6,P1.7
    P1DIR &= ~(BIT2 + BIT3 + BIT4 + BIT5); // ��P1.0��P1.1��P1.6��P1.7��������Ϊ����ģʽ
    P1REN |= BIT2 + BIT3 + BIT4 + BIT5;   // ʹ��P1.0��P1.1��P1.6��P1.7���ŵ���������
    P1OUT |= BIT2 + BIT3 + BIT4 + BIT5;   // ��P1.0��P1.1��P1.6��P1.7���ŵ�������������Ϊ����

    P1IES |= BIT2 + BIT3 + BIT4 + BIT5;   // �����½��ش����ж�
    P1IE |= BIT2 + BIT3 + BIT4 + BIT5;   //�ж�ʹ��
    P1IFG &= ~(BIT2 + BIT3 + BIT4 + BIT5); // ����жϱ�־

    //P1IFG3��P1IFG5��ʼΪ1���ر�
    //P1IFG &=~ BIT3+BIT5;
    __enable_interrupt();
}

/*
//λ�÷ֲ�
    0   1   2
0  BL  CTR  SIN
1 TRI  SQR  SAW
*/
//�ж�EC11����ת�������ڱ��⣬�Աȶȣ��ж��ڵ��ڽϿ죬�ж�����ڷ�Ӧ�ٶۣ�
void EC11_direction()
{
        if((P2IN & BIT4) ? 1 : 0 != (P2IN & BIT5) ? 1 : 0)
    {
        if((P2IN & BIT4) ? 1 : 0==1)
        {
            if(cursor_row==0 && cursor_col==0 && flag_select==1){//BL
            //�Աȶȣ��������
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
            //���ε�������Ƶ
            if(flag_amplitude){
                amplitude_count++;
            }
            if(flag_frequence) frequence_count++;
        }
        else if((P2IN & BIT5) ? 1 : 0==0){
            if(cursor_row==0 && cursor_col==0 && flag_select==1){//BL
            //�Աȶȣ��������
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
            //���ε�������Ƶ
            if(flag_amplitude){
                amplitude_count--;
            }
            if(flag_frequence) frequence_count--;
        }
        flag_count=1;
        while((!((P2IN & BIT4) ? 1 : 0))||(!((P2IN & BIT5) ? 1 : 0)));//AB���ƽ״��,��ͬʱѭ����ת
    }
}
/************************************/


/*****LCD1602*******
 ******************/
//�������Ʋ���
// ����P1IFG�Ĵ�����ֵ
uint8_t p1ifg=0;
void switch_cursor_interrupt()
{ 
    if(flag_select)
    {
        if (p1ifg & BIT4) // P1.4���Ŵ����ж�,�󰴼�
        {
            LCD1602_WriteCMD(0XC0+2);
            flag_amplitude=1;
            flag_frequence=0;
        }
        if (p1ifg & BIT5) // P1.5���Ŵ����жϣ��Ұ���
        {
            LCD1602_WriteCMD(0XC0+8);
            flag_frequence=1;
            flag_amplitude=0;
        }
    }
    else if(flag_select==0)
    {
        if (p1ifg & BIT2) // P1.2���Ŵ����ж�
        {
            cursor_row=0;
        }
        if (p1ifg & BIT3) // P1.3���Ŵ����ж�
        {
            cursor_row=1;
        }
        if (p1ifg & BIT4) // P1.4���Ŵ����ж�,�󰴼�
        {
            cursor_col--;//0,1,2
            if(cursor_col<1) cursor_col=0;
        }
        if (p1ifg & BIT5) // P1.5���Ŵ����жϣ��Ұ���
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
            //�������Ҳ���
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
void switch_EC11_button()//��EC11�ж�PORT2
{

    if(P2IFG & BIT0){
        TA0CCTL0 |=CCIE;
        flag_select=1;
        flag_count =1;
        flag_menu  =0;
        flag_select_count++;
        if(flag_select_count==5) {flag_select=0;flag_menu=1;TA0CCTL0 &=~ CCIE;flag_select_count=0;}//�˳�ѡ�еĲ���
    }
}
//���⣬�Աȶ�
// Backlight:���� P2.4 ����BLACK����PWM������ڱ���
void Backlight_contrast_timer_init()
{
    //����
	P3DIR |= BIT6; //LCD1602�����K
	P3SEL |= BIT6;
	TB0CCR0 = cycle;
    TB0CCR6 = Backlight_duty_cycle; // ռ�ձ�
    TB0CCTL6 = OUTMOD_7; // ���ö�ʱ��A1��CCR2���ģʽΪPWMģʽ

    // contrast:�Աȶ� Vo��ѹ0~1.1V��p3.5
    P3DIR |= BIT5; //LCD1602�����K
    P3SEL |= BIT5;
    TB0CCR0 = cycle;
    TB0CCR5 = contrast_duty_cycle; // ռ�ձ�
    TB0CCTL5 = OUTMOD_7; // ���ö�ʱ��A1��CCR2���ģʽΪPWMģʽ
    TB0CTL = TASSEL_2 + MC_1 + ID_0; // ���ö�ʱ��A1��ʱ��ԴΪSMCLK������ģʽΪUp����Ƶϵ��Ϊ1
}
/************************************/
#endif
