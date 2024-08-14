#include <msp430f5529.h>
//#include <driverlib.h>
#include "lcd1602.h"
#include "EC11_button.h"
#include "TLC5616.h"
#include "timer.h"
#include "py_dac_sinWav.h"
void upVcc(void) {  //核心电压上升3级
    PMMCTL0_H = 0xA5;                      //开启PMM电源管理，即开锁
    SVSMLCTL |= SVSMLRRL_1 + SVMLE;        //配置SVML电压
    PMMCTL0 = PMMPW + PMMCOREV_3;           //配置内核电压，选择3级
    while ((PMMIFG & SVSMLDLYIFG) == 0);      //等待配置完成
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG + SVSMLDLYIFG);
    if ((PMMIFG & SVMLIFG) == 1)
        while ((PMMIFG & SVMLVLRIFG) == 0);
    SVSMLCTL &= ~SVMLE;                    //关闭SVML
    PMMCTL0_H = 0x00;                     //锁存配置，即关锁
}

void timerup(void) {//配置时钟25MHZ
    UCSCTL3 = SELREF_2;
    UCSCTL4 |= SELA_2;
    __bis_SR_register(SCG0);
    UCSCTL0 = 0x0000;
    UCSCTL1 = DCORSEL_7;//50Mhz范围
    UCSCTL2 = FLLD_0 + 762;    //(762+1)*32768==25MHZ
    __bic_SR_register(SCG0);
    __delay_cycles(782000); //延时
    while (SFRIFG1 & OFIFG) {//等待设置完成
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
        SFRIFG1 &= ~OFIFG;
    }
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    //UCSCTL1 = DCORSEL_7; // 设置DCO频率为14MHz
    upVcc();
    timerup();
    /*LCD1602部分*/
    LCD1602_Initi();
    Backlight_contrast_timer_init();//背光 PWM初始化
    LCD1602_WriteCMD(0x80);//书写指令
    DispStr("signal generator"); //开始界面
    LCD1602_WriteCMD(0x80);
    LCD1602_WriteCMD(0X01);
    DispStr("  BL  CTR  SIN ");
    LCD1602_WriteCMD(0XC0);
    DispStr(" TRI  SQR  SAW ");
    /***********/
    //TCL5616_IO_init();//TLC5616初始化
    //init_ref_voltage();//2.5V基准电压初始化
    init_timer();//初始化定时器，用于DDS发出信号
    init_SPI(); // 初始化SPI模块
    IO_Button_init(); //EC11 按键 IO口初始化 ,***此处开了总中断***

    MPY32CTL0 |= OP2_32+OP1_32; //配置硬件乘法器 选择无符号乘法;

    while(1)
    {
        switch_key_while();//是否进入光标所在波形选项
    }
}

// 定时器中断服务函数
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
        index = (index + frequence_count) % 255;//更新相位
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IE &=~ (EC11_PIN_A + EC11_PIN_B+BIT0); //关闭按键中断消抖
    __enable_interrupt();
    
    if((P2IFG & BIT0) ==0)
    EC11_direction();//判断转动方向以及上一次的状态，旋转幅度，调节背光，对比度PWM占空比
    
    switch_EC11_button();//button,判断是否按下，选中当前波形选项

    P2IFG &= ~(EC11_PIN_A + EC11_PIN_B+BIT0); // 清除旋转编码器EC11的中断标志位
    P2IE |= (EC11_PIN_A + EC11_PIN_B+BIT0);//开启按键中断消抖
}

#pragma vector=PORT1_VECTOR // 中断服务函数
__interrupt void Port_1(void)
{
    p1ifg = P1IFG;
    P1IE &=~ BIT2 + BIT3 + BIT4 + BIT5;
    P1IFG = 0;
    __bis_SR_register(GIE);
    switch_cursor_interrupt();//
    // 清除已经处理的中断标志位
    P1IE |= BIT2 + BIT3 + BIT4 + BIT5;
    P1IFG = 0;
}
