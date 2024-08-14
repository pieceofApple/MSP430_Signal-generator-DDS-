void SPI_init()
{
    UCB1CTL1 |= UCSWRST;
    UCB1CTL0  =  0x00+UCMST + UCSYNC + UCMODE_0 + UCMSB + UCCKPH;
    UCB1CTL1 |=  UCSSEL_3;                  //data rate:
    UCB1BR1   =  0x00;
    UCB1BR0   =  0x01;                     // division factor of clock source
      /* 3) Configure ports
       MISO -> P3.1
       MOSI -> P3.0
       SCLK -> P3.2
       CS_N -> manualy set. (XOUT P2.2)*/
      P4SEL = BIT0+ BIT1+BIT2;  

      P4DIR |=BIT3;                             // Configures SPI
      P4OUT &= ~BIT3;                                        //select device
      
      P4OUT |= BIT3;
      P4DIR |= BIT0 + BIT2;                                    // Set as outputs
      P4DIR&=~BIT1;//
      UCB1CTL1 &= ~UCSWRST;                                    //enable module or module initialization
      P4OUT&=~BIT3;//Make CS Low
}