#include "s3c24xx.h"

#define TXD0READY   (1<<2)
#define RXD0READY   (1)
#define PCLK            50000000    
#define UART_CLK        PCLK        
#define UART_BAUD_RATE  115200      
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)


void uart0_init()
{
    GPHCON  |= 0xa0;    // GPH2,GPH3 --> TXD0,RXD0    
    GPHUP   = 0x0c;     // GPH2,GPH3 pull up disable 
    ULCON0  = 0x03;     // 8N1( 
    UCON0   = 0x05;     // Interrupt request or polling mode
    UFCON0  = 0x00;     // no FIFO    
    UMCON0  = 0x00;     // no flow control    
    UBRDIV0 = UART_BRD; // bandrate 115200

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

