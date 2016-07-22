#include "s3c24xx.h"

#define	GPF4_out	(1<<(4*2))
#define	GPF5_out	(1<<(5*2))
#define	GPF6_out	(1<<(6*2))

void led_init()
{
	GPFCON = GPF4_out|GPF5_out|GPF6_out;
	GPFDAT |= (0x7<<4); //close all led       
}

int led_on(int num)
{

  switch(num)
  {
	case 1:
		GPFDAT &= ~(1<<4);      
		break;
	case 2:
		GPFDAT &= ~(1<<5);      
		break;
	case 3:
		GPFDAT &= ~(1<<6);     
		break;
	default:
		return 0;
  }

  return num;
}


int led_off(int num)
{

  switch(num)
  {
	case 1:
		GPFDAT |= (1<<4);      
		break;
	case 2:
		GPFDAT |= (1<<5);      
		break;
	case 3:
		GPFDAT |= (1<<6);
		break;
	default:
		return 0;
  }

  return num;
}
