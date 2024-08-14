#include <msp430f5529.h>
//#include <driverlib.h>
#include "lcd1602.h"
#include "EC11_button.h"
#include "TLC5616.h"
#include "timer.h"
#include "py_dac_sinWav.h"
void upVcc(void) {  //���ĵ�ѹ����3��
    PMMCTL0_H = 0xA5;                      //����PMM��Դ����������
    SVSMLCTL |= SVSMLRRL_1 + SVMLE;        //����SVML��ѹ
    PMMCTL0 = PMMPW + PMMCOREV_3;           //�����ں˵�ѹ��ѡ��3��
    while ((PMMIFG & SVSMLDLYIFG) == 0);      //�ȴ��������
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG + SVSMLDLYIFG);
    if ((PMMIFG & SVMLIFG) == 1)
        while ((PMMIFG & SVMLVLRIFG) == 0);
    SVSMLCTL &= ~SVMLE;                    //�ر�SVML
    PMMCTL0_H = 0x00;                     //�������ã�������
}

void timerup(void) {//����ʱ��25MHZ
    UCSCTL3 = SELREF_2;
    UCSCTL4 |= SELA_2;
    __bis_SR_register(SCG0);
    UCSCTL0 = 0x0000;
    UCSCTL1 = DCORSEL_7;//50Mhz��Χ
    UCSCTL2 = FLLD_0 + 762;    //(762+1)*32768==25MHZ
    __bic_SR_register(SCG0);
    __delay_cycles(782000); //��ʱ
    while (SFRIFG1 & OFIFG) {//�ȴ��������
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
        SFRIFG1 &= ~OFIFG;
    }
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    //UCSCTL1 = DCORSEL_7; // ����DCOƵ��Ϊ14MHz
    upVcc();
    timerup();
    /*LCD1602����*/
    LCD1602_Initi();
    Backlight_contrast_timer_init();//���� PWM��ʼ��
    LCD1602_WriteCMD(0x80);//��дָ��
    DispStr("signal generator"); //��ʼ����
    LCD1602_WriteCMD(0x80);
    LCD1602_WriteCMD(0X01);
    DispStr("  BL  CTR  SIN ");
    LCD1602_WriteCMD(0XC0);
    DispStr(" TRI  SQR  SAW ");
    /***********/
    //TCL5616_IO_init();//TLC5616��ʼ��
    //init_ref_voltage();//2.5V��׼��ѹ��ʼ��
    init_timer();//��ʼ����ʱ��������DDS�����ź�
    init_SPI(); // ��ʼ��SPIģ��
    IO_Button_init(); //EC11 ���� IO�ڳ�ʼ�� ,***�˴��������ж�***

    MPY32CTL0 |= OP2_32+OP1_32; //����Ӳ���˷��� ѡ���޷��ų˷�;

    while(1)
    {
        switch_key_while();//�Ƿ���������ڲ���ѡ��
    }
}

// ��ʱ���жϷ�����
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void)
{
        static unsigned int index = 0;
        unsigned long int result=0,result_H=0;
        if(cursor_row==0 && cursor_col==2)
        MPY32L = sin_table_256[index];
        if(cursor_row==1 && cursor_col==0)
        MPY32L = tri_table_256[index];
        if(cursor_row==1 && cursor_col==1)
        MPY32L = sqr_table_256[index];
        if(cursor_row==1 && cursor_col==2)
        MPY32L = saw_table_256[index];

            OP2L = proportion[amplitude_count];
            result_H=RES1;
            result_H<<=16;
            result = RES0+result_H;
            result>>=8;
        P2OUT &=~BIT7;
        write_TLC5616(result);
        P2OUT |=BIT7;
        //DA_OUTPUT(result);
        index = (index + frequence_count) % 255;//������λ
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IE &=~ (EC11_PIN_A + EC11_PIN_B+BIT0); //�رհ����ж�����
    __enable_interrupt();
    
    if((P2IFG & BIT0) ==0)
    EC11_direction();//�ж�ת�������Լ���һ�ε�״̬����ת���ȣ����ڱ��⣬�Աȶ�PWMռ�ձ�
    
    switch_EC11_button();//button,�ж��Ƿ��£�ѡ�е�ǰ����ѡ��

    P2IFG &= ~(EC11_PIN_A + EC11_PIN_B+BIT0); // �����ת������EC11���жϱ�־λ
    P2IE |= (EC11_PIN_A + EC11_PIN_B+BIT0);//���������ж�����
}

#pragma vector=PORT1_VECTOR // �жϷ�����
__interrupt void Port_1(void)
{
    p1ifg = P1IFG;
    P1IE &=~ BIT2 + BIT3 + BIT4 + BIT5;
    P1IFG = 0;
    __bis_SR_register(GIE);
    switch_cursor_interrupt();//
    // ����Ѿ�������жϱ�־λ
    P1IE |= BIT2 + BIT3 + BIT4 + BIT5;
    P1IFG = 0;
}
