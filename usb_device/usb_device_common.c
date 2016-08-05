#include "include.h"
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

void RdPktEp3(U8 *buf,int num)
{
    int i;
    	
    for(i=0;i<num;i++)
    {
        buf[i]=(U8)EP3_FIFO;	
    }
}

void WrPktEp1(U8 *buf,int num)
{
    int i;
    	
    for(i=0;i<num;i++)
    {
        EP1_FIFO=buf[i];	
    }
}


void ClearEp3OutPktReady()
{
    U8 out_csr3;

    INDEX_REG=3;
    out_csr3= OUT_CSR1_REG;
    CLR_EP3_OUT_PKT_READY();
}

//close ep3 dma interrupt
void ConfigEp3IntMode()
{
    INDEX_REG=3;
    
    DMASKTRIG2= (0<<1);  // EP3=DMA ch 2
    //DMA channel off
    OUT_CSR2_REG=OUT_CSR2_REG&~(EPO_AUTO_CLR/*|EPO_OUT_DMA_INT_MASK*/); 
    //AUTOCLEAR off,interrupt_enabled (???)
    EP3_DMA_UNIT=1;	
    EP3_DMA_CON=0; 
    // deamnd disable,out_dma_run=stop,in_dma_run=stop,DMA mode disable
    //wait until DMA_CON is effective.
    EP3_DMA_CON;
    
}


//open ep3 dma interrupt
void ConfigEp3DmaMode(unsigned int bufAddr,unsigned int count)
{

    printf("addr : 0x%x,count:%d\r\n", bufAddr,count);
    count = count&0xfffff; 	//transfer size should be <1MB
    OUT_CSR2_REG = OUT_CSR2_REG|EPO_AUTO_CLR|EPO_OUT_DMA_INT_MASK;  //EPO_AUTO_CLR need do here OUT_PKT_RDY auto clear used in dma

	INDEX_REG=3;
	DISRCC2 = (1<<1)|(1<<0); //increased source is APB
    DISRC2 = 0x520001cc; 	//src=APB,fixed,src=EP3_FIFO
    DIDSTC2=(0<<1)|(0<<0);  //fixed dest is AHB
    DIDST2=bufAddr;       	//dst=bufAddr
    DCON2=(count)|((unsigned int)1<<31)|(0<<30)|(1<<29)|(0<<28)|(0<<27)|(4<<24)|(1<<23)|(0<<22)|(0<<20); 
	
    EP3_DMA_UNIT = 0x01; //DMA transfer unit=1byte
    EP3_DMA_CON = UDMA_OUT_DMA_RUN|UDMA_DMA_MODE_EN;  //ep3 dma mode  run

	DMASKTRIG2= (1<<1);    //start dma2

}

