#if !defined(TLC5616_H)
#define TLC5616_H

#include <msp430.h>
//FS     Ƭѡ�˿�P4.4
//CLK    ʱ�Ӷ˿�P4.3
//DIN    ��������P4.1
//CS	 		 GND
//Ӳ��SPI
void init_SPI()
{
    //STE
    P2SEL &= ~BIT7;
    P2DIR |= BIT7;
    P2OUT |= BIT7;
//    P2SEL &=~BIT3;
//    P2DIR |=BIT3;
//    P2OUT |=BIT3;
    // ����UCB0ģ��Ķ˿�
    P4SEL |= BIT1 +BIT2+BIT3;

    // ����UCB1ģ��
    UCB1CTL1 |= UCSWRST+UCSSEL_2; // ����SPI�����λ״̬//data rate
    UCB1CTL0 |= UCMSB | UCMST | UCSYNC; // ����SPIģ��Ŀ��ƼĴ���������Ϊ3��ģʽ����ģʽ��ʱ����λΪ�ڶ������ز�����MSB���ȣ�ͬ��ģʽ
    UCB1CTL0 &=~ UCCKPH+UCCKPL;
    UCB1BR0 |= 0x02; // ����SPIʱ���ٶ�ΪSMCLK/2
    UCB1BR1 = 0;
    UCB1CTL1 &= ~UCSWRST; // �˳�SPI�����λ״̬
}
void init_ref_voltage() {
    P5SEL |= BIT0;
    // ����REFģ��Ϊʹ���ⲿ�ο���ѹ���ο���ѹΪ2.5V
    REFCTL0 &= ~REFMSTR; // �������ο�Դ��ʹ��
    REFCTL0 |= REFVSEL_2 | REFOUT; // ѡ��2.5V�ο���ѹ��ʹ��REFOUT���
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
//Ӳ��SPI
void write_TLC5616(unsigned int data)
{
    UCB1TXBUF = data >> 8; // ����8λ����д��TX������
    while (!(UCB1IFG & UCTXIFG)); // �ȴ����ݷ������

    UCB1TXBUF = data & 0xFF; // ����8λ����д��TX������
    while (!(UCB1IFG & UCTXIFG)); // �ȴ����ݷ������
}
//���SPI
void DA_OUTPUT(unsigned int value)
{
	unsigned char i;  //�����������ѭ��
	//value<<=4;        //int����Ϊ16λ������12λ��ȥ����4λ����ȡ��
    P4OUT |=  BIT3;    //ʱ��CLK=1;     
	P4OUT &=~ BIT4;    //FS=0;

	/*ѭ��16��ȡ��16λд��ģ��Ĵ���*/
	for(i = 0; i < 16; i++)
	{
		if(value & 0X8000) P4OUT |= BIT1;
		else P4OUT &=~ BIT1;
        P4OUT &=~ BIT3;//CLK=1;
		value<<=1;
		P4OUT |= BIT3;//CLK=0;
	}
	P4OUT |= BIT3;//CLK=1;             //�ָ�Ĭ�ϵ͵�ƽ״̬
	P4OUT |= BIT4;//FS=1;
}
#endif // TLC5616_H
