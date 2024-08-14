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

//74HC164����������P3.7-CLK��P2.3-data
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
        CLR_RS;     //ָ��
        HC164_Data(cmd);       //ָ�����ݴ���P0�ڴ����͸�LCD1602
        delay_ms(1);
        SET_EN;     //LCD1602ʹ�ܿ�
        delay_ms(10);
        CLR_EN;     //�����½���
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
        P3DIR |= BIT7;//74HC164��clk
        P8DIR |= BIT1 + BIT2;//P8.1----RS,P8.2---EN
        P2DIR |= BIT3 + BIT6;//P2.3---164input;P2.6---VL
        LCD1602_WriteCMD(0x38); //  �ù��ܣ�8λ���ߣ�˫����ʾ����ʾ5X7�ĵ����ַ�
        LCD1602_WriteCMD(0x0f); //  ��ʾ���ؿ���,����ʾ, �й��,��˸
        LCD1602_WriteCMD(0x06); //  ������ʾλ������,  ����ƶ���������,��Ļ���������ֲ���
        LCD1602_WriteCMD(0x01); //  ����ʾ��ָ����01H����긴λ����ַ00Hλ��
        delay_ms(10);
    }
//VL�ŵ�PWM������
void PWM_lcd(void) {
    //PWM
    P3DIR |= BIT5 + BIT6;  //TB0.5,���PWM���ź�
    P3SEL |= BIT5 + BIT6;
    //P3OUT |= BIT6;
    TB0CTL |= TBSSEL_1 + MC_1;//��ʱ��TB0��ʱ��Դѡ��ACLK�����ϼ�����
    TB0CCTL0 |= CAP;//��0��ͨ��Ϊ �Ƚ�ģʽ

    TB0CCTL5 |= CAP;//��5��ͨ��Ϊ �Ƚ�ģʽ����ʵ����Ĭ�Ͼ��ǱȽ�ģʽ��pwm��׽����ʱ��
    TB0CCTL5 = OUTMOD_7;//�����ͨ������Ϊ���ģʽ������PWM�����ź�

    TB0CCTL6 |= CAP;//��6��ͨ��Ϊ �Ƚ�ģʽ����ʵ����Ĭ�Ͼ��ǱȽ�ģʽ��pwm��׽����ʱ��
    TB0CCTL6 = OUTMOD_7;//������ͨ������Ϊ���ģʽ������PWM�����ź�

    TB0CCR0 = 800;
    TB0CCR5 = 350;//δ��ǰĬ����ʾ1800
    TB0CCR6 = 900;//δ��ǰĬ����ʾ2000
}
