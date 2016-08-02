#include "s3c24xx.h"
#include "usb_device_common.h"

void usb_clk_init()
{
    UPLLCON = (0x38<<12)|(0x02<<4)|(0x02);	//Uclk = 48Mhz
}

void usb_dev_enable()
{
    MISCCR &= ~(1<<3);		// usb1 as device,not host
    MISCCR &= ~(1<<13);		// enable usb1
}

void usb_dev_port_init()
{
	GPCCON &= ~(3<<10);		// GPC5 clear
    GPCCON |= 1<<10;		// GPC5 output
    GPCUP |= 1<<5;			// pullup disable
    GPCDAT |= 1<<5;			// high lever output
}

void WrPktEp0(U8 *buf,int num)
{
    int i;
    	
    for(i=0;i<num;i++)
    {
    	EP0_FIFO=buf[i];	
    }
}

void RdPktEp0(U8 *buf,int num)
{
    int i;
    	
    for(i=0;i<num;i++)
    {
        buf[i]=(U8)EP0_FIFO;	
    }
}
