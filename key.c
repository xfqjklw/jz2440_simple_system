#include "s3c24xx.h"

#define GPF0_eint     (0x2<<(0*2))
#define GPF2_eint     (0x2<<(2*2))
#define GPG3_eint     (0x2<<(3*2))
#define GPF0_msk      (3<<(0*2))
#define GPF2_msk      (3<<(2*2))
#define GPG3_msk      (3<<(3*2))

void key_init()
{
	//set gpio EINT0,ENT2
	GPFCON &= ~(GPF0_msk | GPF2_msk);    
	GPFCON |= GPF0_eint | GPF2_eint;

	//set gpio EINT11
	GPGCON &= ~GPG3_msk;
	GPGCON |= GPG3_eint;

	// EINT11£¬need set INTMASK
	EINTMASK &= ~(1<<11);

	//EINT0¡¢EINT2¡¢EINT8_23 enable
	INTMSK   &= (~(1<<0)) & (~(1<<2)) & (~(1<<5));

	EXTINT0 |= ((0x02) | (0x02<<8)); //fall edge 
}


