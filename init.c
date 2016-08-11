#include "s3c24xx.h"

#define S3C2440_MPLL_400MHZ		((0x5c<<12)|(0x01<<4)|(0x01))
#define S3C2440_MPLL_200MHZ     ((0x5c<<12)|(0x01<<4)|(0x02))

void clock_init(void)
{
    CLKDIVN  = 0x05;            // FCLK:HCLK:PCLK=1:4:8, HDIVN=2,PDIVN=1

    /* if HDIVN not 0£¬change fast bus mode to synchronous bus mode */
__asm__(
    "mrc    p15, 0, r1, c1, c0, 0\n"      
    "orr    r1, r1, #0xc0000000\n"          
    "mcr    p15, 0, r1, c1, c0, 0\n"
    );

    MPLLCON = S3C2440_MPLL_400MHZ;  /* FCLK=400MHz,HCLK=100MHz,PCLK=50MHz */
}

/*
 * sdram setup
*/
void memsetup(void)
{
    volatile unsigned long *p = (volatile unsigned long *)MEM_CTL_BASE;


    p[0] = 0x22011110;     //BWSCON
    p[1] = 0x00000700;     //BANKCON0
    p[2] = 0x00000700;     //BANKCON1
    p[3] = 0x00000700;     //BANKCON2
    p[4] = 0x00000700;     //BANKCON3  
    p[5] = 0x00000700;     //BANKCON4
    p[6] = 0x00000700;     //BANKCON5
    p[7] = 0x00018005;     //BANKCON6
    p[8] = 0x00018005;     //BANKCON7
    
	/* REFRESH,
	 * HCLK=12MHz:  0x008C07A3,
	 * HCLK=100MHz: 0x008C04F4
	 */ 
    p[9]  = 0x008C04F4;
    p[10] = 0x000000B1;     //BANKSIZE
    p[11] = 0x00000030;     //MRSRB6
    p[12] = 0x00000030;     //MRSRB7
}

/*
 * copy code from code to sdram
*/
void copy_nand_to_sdram()
{
	unsigned int *Dest = (unsigned int *)0x30000000;

	nand_read(Dest,0,2048*15);
	
}

void copy_steppingstone_to_sdram()
{    
	unsigned int *pdwSrc  = (unsigned int *)0;    
	unsigned int *pdwDest = (unsigned int *)0x30000000;        
	while (pdwSrc < (unsigned int *)4096)    
	{        
		*pdwDest = *pdwSrc;        
		pdwDest++;        
		pdwSrc++;    
	}
}

/*
*if you want to debug in sdram,you should copy isr talbe to 0x00~0x20
*/
void copy_isr_talbe_to_sram()
{
	unsigned int *pdwSrc  = (unsigned int *)0x30000000;
	unsigned int *pdwDest = (unsigned int *)0;
	while (pdwSrc < (unsigned int *)0x30000040)
	{        
		*pdwDest = *pdwSrc;        
		pdwDest++;        
		pdwSrc++;    
	}
}

/*
*you should clean bss,if you don't do this,the global variable which set 0 don't be 0
*/
void clean_bss(void)
{
    extern int __bss_start, __bss_end;
    int *p = &__bss_start;
    
    for (; p < &__bss_end; p++)
        *p = 0;
}