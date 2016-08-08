#include "include.h"
#include "s3c24xx.h"

#define GPE0_I2SLRCK (0x2<<(2*0))
#define GPE1_I2SSCKL (0X2<<(2*1))
#define GPE2_CDCLK   (0X2<<(2*2))
#define GPE3_I2SDI   (0x2<<(2*3))
#define GPE4_I2SDO   (0x2<<(2*4))

#define GPE0_NOT_PULL_UP ((0x1<<0))
#define GPE1_NOT_PULL_UP ((0x1<<1))
#define GPE2_NOT_PULL_UP ((0x1<<2))
#define GPE3_NOT_PULL_UP ((0x1<<3))
#define GPE4_NOT_PULL_UP ((0x1<<4))

#define GPB4_OUT (0x1<<(2*4))
#define GPB3_OUT (0x1<<(2*3))
#define GPB2_OUT (0x1<<(2*2))

#define IISCON_PRESCALER_ENABLE (0x1<<1)
#define IISCON_INTERFACE_ENABLE (0x1<<0)
#define IISCON_RX_IDLE (0x1<<2)
#define IISCON_TX_IDLE (0x1<<3)
#define IISCON_TX_DMA (0x1<<5)
#define IISCON_ENABLE_IIS (0x1<<0)

#define IISMOD_SCLK_32FS (0x1<<0)
#define IISMOD_MCLK_384FS (0x1<<2)
#define IISMOD_MCLK_256FS (0x0<<2)
#define IISMOD_SERIAL_BIT_PER_CH_16 (0x1<<3)
#define IISMOD_MSB_FORMAT (0x1<<4)
#define IISMOD_LOW_FOR_LEFT_CH (~(0x0<<5))
#define IISMOD_TXMOD (0x2<<6)

#define IISFCON_TX_ENABLE (0x1<<13)
#define IISFCON_RX_FIFO_DMA (0x1<<14)
#define IISFCON_TX_FIFO_DMA (0x1<<14)

#define IS_FIFO_READY (0x1<<7)

#define GPB_X_WRITE(x) (1<<x)
#define GPX_X_CLEAR(x) (~(1<<x))


static void udelay(int time)
{
	while(time--);
}


static void s3c2440_iis_init()
{
	GPECON = GPECON
			|GPE0_I2SLRCK
			|GPE1_I2SSCKL
			|GPE2_CDCLK
			|GPE3_I2SDI
			|GPE4_I2SDO;

	GPEUP = GPEUP
			|GPE0_NOT_PULL_UP
			|GPE1_NOT_PULL_UP
			|GPE2_NOT_PULL_UP
			|GPE3_NOT_PULL_UP
			|GPE4_NOT_PULL_UP;


	/*
	*i2s prescaler enable,i2s enable
	*/	
	IISCON = 0;
	IISCON |= IISCON_PRESCALER_ENABLE 
	 		//|IISCON_TX_DMA
	 		|IISCON_ENABLE_IIS;


	/*
	*master clock set PCLK
	*master mode
	*tx mode
	*iis format
	*16bit per channel
	*MCLK = 384fs
	*BLCK = 32fs
	*/
	IISMOD = 0;
	IISMOD =IISMOD_SCLK_32FS
			|IISMOD_MCLK_384FS
			|IISMOD_SERIAL_BIT_PER_CH_16
			|IISMOD_TXMOD;

	/*
	*tx fifo enable
	*/
	IISFCON = 0;
	IISFCON =  IISFCON_TX_ENABLE;
	// |IISFCON_RX_FIFO_DMA
	// |IISFCON_TX_FIFO_DMA; 

	/*
	clock set
	fs = 22.050KHZ
	MCLK = 22050*384 = 8.4672MHZ --> MCLK ＝PCLK / (N＋1) --> N = 5
	BCLK = 2*fs*16 = 32fs
	LRCLK = fs
	*/
	IISPSR = (5<<5)|5; 
	
}

static void s3c2440_l3_init()
{
	GPBCON |= GPB2_OUT |GPB3_OUT|GPB4_OUT;
}

static void wm8976_write_reg(unsigned char reg, unsigned int data)
{
    int i;
    unsigned short val = (reg << 9) | (data & 0x1ff);  //9~15 control register address 0~8 control register data

    GPBDAT |= GPB_X_WRITE(2);//CSB
    GPBDAT |= GPB_X_WRITE(3);//CLK
    GPBDAT |= GPB_X_WRITE(4);//DATA

    for (i = 0; i < 16; i++)
	{
        if (val & (1<<15))
        {
            GPBDAT &= GPX_X_CLEAR(4); //set data
            GPBDAT |= GPB_X_WRITE(3); 
            udelay(10);
            GPBDAT |= GPB_X_WRITE(4); //one clk
            //udelay(10);
        }
        else
        {
            GPBDAT &= GPX_X_CLEAR(4);
            GPBDAT &= GPX_X_CLEAR(3);
            udelay(10);
            GPBDAT |= GPB_X_WRITE(4);
            //udelay(10);
        }

        val = val << 1;
    }

    GPBDAT &= GPX_X_CLEAR(2); 
    udelay(10);
    GPBDAT |= GPB_X_WRITE(2);  //csb from low to high one cmd send
    GPBDAT |= GPB_X_WRITE(3);
    GPBDAT |= GPB_X_WRITE(4);
}

void wm8976_reg_init()
{
    /* software reset */
    wm8976_write_reg(0, 0);

  	
    wm8976_write_reg(0x3, 0x6f);/* OUT2的左/右声道打开     * 左/右通道输出混音打开     * 左/右DAC打开*/
    wm8976_write_reg(0x1, 0x1f);//biasen,BUFIOEN.VMIDSEL=11b  
    wm8976_write_reg(0x2, 0x185);//ROUT1EN LOUT1EN, input PGA enable ,ADC enable

    wm8976_write_reg(0x6, 0x0);//SYSCLK=MCLK divide by 1
    wm8976_write_reg(0x4, 0x10);//16bit i2s format
    wm8976_write_reg(0x2B,0x10);//BTL OUTPUT  
    wm8976_write_reg(0x9, 0x50);//Jack detect enable  gpio2 as jack detect input
    wm8976_write_reg(0xD, 0x21);//Jack decet pin is 0,OUT1 enable,1 OUT2 enable
    wm8976_write_reg(0x7, 0x01);//Jack detect slow clock enabled used for jack detect
    wm8976_write_reg(52, (1<<8)|57); //0db LOUT1 VOL
    wm8976_write_reg(53, (1<<8)|57); //0db ROUT1	VOL
	
}

void wm8976_init()
{
	s3c2440_iis_init();
	s3c2440_l3_init();
	wm8976_reg_init();
}

void music_play()
{	
	printf("music_play\r\n");
	nand_init();

	short *pData = (short *)0x33000000;
	short *pData_end = pData + 882046/4;
    int send_cnt = 0;
	int i = 0;

	printf("read start\r\n");
	nand_read((unsigned char *)pData, 0x60000, 0x200000);
	printf("the head of wav : 0x%x\r\n", *pData);
	pData += 0x2e;//real data offset
	
	while(1)
	{
		while (IISCON & IS_FIFO_READY);
		IISFIFO = *pData;
		if (pData == pData_end) 
		{
			return 0;
		}		
		pData++;
}
}
