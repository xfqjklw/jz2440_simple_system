#include "include.h"
#include "s3c24xx.h"


#define TXD0READY   (1<<2)
#define RXD0READY   (1)
#define PCLK            50000000    
#define UART_CLK        PCLK        
#define UART_BAUD_RATE  115200      
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)

#if UART_SEND_DMA

int uartSendDmaEnd = 1;
char *uartSendBuffer = "this is uart dma test\r\n";

void uart0_dma_send_init()
{	
	DISRC0 = uartSendBuffer;
	DISRCC0 |= (0<<1)|(0<<0);  //bit0:0=increase 1=fixed 
								//bit1:0: the source is in the system bus (AHB).
								//     1: the source is in the peripheral bus (APB).
								//source is uartSendBuffer(sdran AHB)
	DIDST0 = 0x50000020;		//don't use UTX0 notice!!!!!!!!!!!!!!!!!!
	DIDSTC0 |=(0<<2)|(1<<1)|(1<<0);  // fixed APB  Interrupt will occur when TC reaches 0

	DCON0 |= (1<<31)|(0<<30)|(1<<29)|(0<<28)|(0<<27)|(1<<24)|(1<<23)|(1<<22)|(0<<20)|(24);
	//31:Handshake mode will be selected.
	//30:DREQ and DACK are synchronized to PCLK (APB clock). 
	//29:Enable/Disable the interrupt setting for CURR_TC 1:enable
	//28:Select the transfer size of an atomic transfer 0:A unit transfer is performed 1:A burst transfer of length four is performed.
	//27: Single service mode is selected in which after each atomic transfer (single or burst of length four) DMA stops and waits for another DMA request.
	//24~26:001:UART0
	//23:DMA source selected by bit[26:24] triggers the DMA operation.
	//22:DMA channel (DMA REQ) is turned off when a current value of transfer count becomes 0. 
	//0~19:initial transfer count

	INTMSK &=~(1<<17);	//open the dma0 interrupt
			
}

void uart0_dma_send_start()
{
	if(uartSendDmaEnd == 1)
	{
		uartSendDmaEnd = 0;
		DMASKTRIG0 = (0<<2)|(1<<1)|(0<<0);  //dma0 turn on
	}
}

#endif

void uart0_init()
{
    GPHCON |= 0xa0;    // GPH2,GPH3 --> TXD0,RXD0    
    GPHUP   = 0x00;     // GPH2,GPH3 pull up disable 
    ULCON0  = 0x03;     // 8N1(     
	#if UART_SEND_DMA
	UCON0	|=((1<<0) | (1<<3) | (2<<10)); //used pclk as the clock,  transmit use DMA mode  ,  receive use  polling mode    
	#else
	UCON0   = 0x05;     // Interrupt request or polling mode
	#endif
	UFCON0  = 0x00;     // no FIFO    
    UMCON0  = 0x00;     // no flow control
    UBRDIV0 = UART_BRD; // bandrate 115200

	#if UART_SEND_DMA
	uart0_dma_send_init();
	#endif
}

void putc(unsigned char c)
{
	while(!(UTRSTAT0 & TXD0READY));
	UTXH0 = c;
}

unsigned char getc()
{
	while(!(UTRSTAT0 & RXD0READY));
	return URXH0;
}

