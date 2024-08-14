void flag_judge() {
    int flag_bits = 0;  // 定义标志位的32位整数类型
    // 使用位运算代替数组访问
    if (flag_bits & (1 << 0)) {  // BL背光
        LCD1602_WriteCMD(0XC0);
        LCD1602_Writenumber(Backlight_duty_cycle);
    }

    if (flag_bits & (1 << 1)) {  // CTR对比度
        LCD1602_WriteCMD(0XC0 + 11);
        LCD1602_Writenumber(contrast_duty_cycle);
    }

    if (flag_bits & (1 << 2)) {  // SIN正弦波
        LCD1602_WriteCMD(0XC0);
        DispStr("AMP:");
        SIN_amplitude = (5.5 - SIN_amplitude_count * 0.5) * 10;
        LCD1602_WriteCMD(0XC0 + 4);
        LCD1602_Writenumber(SIN_amplitude);
        LCD1602_WriteCMD(0XC0 + 8);
        DispStr("FRE:");
        LCD1602_Writenumber(SIN_frequency);
    }

    if (flag_bits & (1 << 3) || flag_bits & (1 << 4) || flag_bits & (1 << 5)) {  // TRI, SQR, SAW
        LCD1602_WriteCMD(0XC0);
        DispStr("AMP");
        LCD1602_WriteCMD(0XC0 + 8);
        DispStr("FRE");
    }

    if (flag_bits & (1 << 6)) {  // AMP
        // ...
    }

    if (flag_bits & (1 << 7)) {  // FRE
        // ...
    }
}


/******被优化的标志位原代码***/
#if !defined(FLAG_H)
#define FLAG_H
#include "EC11_button.h"

void flag_clear()
{
    int i=0;
    for(i=0;i<8;i++)
    {
        flag_menu[i]=0;
    }
}
void flag_judge()
{
    int i=0;
    for(i=0;i<8;i++)
    {
        if(flag_menu[i]==1)
        {
            //int j = 0;
            switch (i)
            {
            case 0://BL背光
                LCD1602_WriteCMD(0XC0);
                LCD1602_Writenumber(Backlight_duty_cycle);
                break;
            case 1://CTR对比度
                LCD1602_WriteCMD(0XC0+11);
                LCD1602_Writenumber(contrast_duty_cycle);
                break;
            case 2://SIN正弦波
                LCD1602_WriteCMD(0XC0);
                DispStr("AMP:");
                SIN_amplitude = (5.5-SIN_amplitude_count*0.5)*10;
                LCD1602_WriteCMD(0XC0+4);
                LCD1602_Writenumber(SIN_amplitude);
                LCD1602_WriteCMD(0XC0+8);
                DispStr("FRE:");
                LCD1602_Writenumber(SIN_frequency);
                break;
            case 3://TRI三角波
                LCD1602_WriteCMD(0XC0);
                DispStr("AMP");
                //LCD1602_Writenumber(SIN_amplitude);
                LCD1602_WriteCMD(0XC0+8);
                DispStr("FRE");
                //LCD1602_Writenumber(SIN_frequency);
                break;
            case 4://SQR方波
                LCD1602_WriteCMD(0XC0);
                DispStr("AMP");
                //LCD1602_Writenumber(SIN_amplitude);
                LCD1602_WriteCMD(0XC0+8);
                DispStr("FRE");
                //LCD1602_Writenumber(SIN_frequency);
            case 5://SAW锯齿波
                LCD1602_WriteCMD(0XC0);
                DispStr("AMP");
                //LCD1602_Writenumber(SIN_amplitude);
                LCD1602_WriteCMD(0XC0+8);
                DispStr("FRE");
                //LCD1602_Writenumber(SIN_frequency);
                break;
            case 6://AMP
                /* code */
                break;
            case 7://FRE
                /* code */
                break;
            default:
                break;
            }
            //count=0;
            break;
        }
    }
}
#endif // FLAG_H