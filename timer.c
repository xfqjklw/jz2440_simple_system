#include "s3c24xx.h"

void pwm0_init()
{
	GPBCON &= ~(0x03);
	GPBCON |= 0x02;
		
	TCFG0 = 9;   		 //prescaler 0
	TCFG1 |= 0x03;  	 //timer0 1/16
	TCNTB0 = 3125;      
	TCMPB0 = 3125/3;
	
	TCON |= (0x01<<1);  //Update TCNTB0 & TCMPB0
	TCON &= ~(0x01<<1); //nedd clear 0
	TCON |= 0x09;  //Start for Timer 0  Interval mode (auto reload)
}

void timer0_init()
{

	TCFG0 = 99;   		 //prescaler 0
	TCFG1 |= 0x03;  	 //timer0 1/16
	TCNTB0 = 31250;      //50MHZ/(99+1)/16 = 31250HZ
	
	TCON |= (0x01<<1);  //Update TCNTB0 & TCMPB0
	TCON &= ~(0x01<<1); //nedd clear 0
	TCON |= 0x09;  //Start for Timer 0  Interval mode (auto reload)

	INTMSK &= (~(1<<10));

}

void timer1_init()
{
	TCFG0 = 99;   		 //prescaler 0
	TCFG1 |= (0x03<<4);  //timer1 1/16
	TCNTB1 = 31250;      //50MHZ/(99+1)/16 = 31500HZ

	TCON |= (0x01<<9);  //Update TCNTB1 & TCMPB1	
	TCON &= ~(0x01<<9);	//need clear 0
	TCON |= (0x09<<8);  //Start for Timer 1  Interval mode (auto reload)

	INTMSK &= (~(1<<11));
	
}
