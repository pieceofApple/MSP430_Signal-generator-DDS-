#include <msp430.h>

void init_timer(void)
{
    TA0CCTL0 &=~ CCIE; // �ȴ���Ҫ����ʱ�ٿ���
    TA0CTL = TASSEL_2 + MC_1 + ID_0; // ѡ�� SMCLK ��Ϊʱ��Դ������ģʽΪ����������Ƶϵ��Ϊ 8
    TA0CCR0 = 400 - 1; // ������������ 32767���� 1 ��
}

