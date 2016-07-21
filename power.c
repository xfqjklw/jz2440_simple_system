#include "s3c24xx.h"

void slow_mode_enter()
{
	CLKSLOW |= (1<<4);	
	CLKSLOW |= (1<<5);	

}

void slow_mode_exit()
{
	CLKSLOW &= ~(1<<4);
	CLKSLOW &= ~(1<<5);	

}

/*when enter idle mode,please setup external interrupt to wake up*/
void idle_mode_enter()
{
	CLKCON |= (1<<2);
}
