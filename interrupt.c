#include "include.h"
#include "s3c24xx.h"

void EINT0_ISR()
{
	GPFDAT |= (0x7<<4); //close all led  
	GPFDAT &= ~(1<<4);  //opem led1  
}


void EINT2_ISR()
{
	GPFDAT |= (0x7<<4); //close all led       
	GPFDAT &= ~(1<<5);  //open led2
}


void EINT8_23_ISR()
{	
	if(EINTPEND & (1<<11))
	{
		GPFDAT |= (0x7<<4); // close all led            
		GPFDAT &= ~(1<<6);  // open led3     	
		//clear interrupt
		EINTPEND = (1<<11);   // EINT8_23ºÏÓÃIRQ5	
	}
}

#if UART_SEND_DMA
extern int uartSendDmaEnd;
#endif

#if UART_RECV_DMA
extern char uartRecvBuffer[17];
#endif


void DMA0_ISR()
{
	#if UART_SEND_DMA
	uartSendDmaEnd = 1;
	printf("uart dma send end\r\n");
	#endif
	#if UART_RECV_DMA	
	printf("uart dma recv\r\n");
	printf("%s\r\n",uartRecvBuffer);
	#endif
}

void UART0_ISR()
{
	#if UART_FIFO_RECV_INT
	char buff[64];
	char *ps = buff;
	int i = 0;
	
	i = (UFSTAT0&0x1f);
 	printf("count is %d\r\n",i);
	
	if(SUBSRCPND & 0x01)  //recv interrupt
	{
		while(((UFSTAT0&0x1f)>0))
		{
			*ps++ = URXH0;
		}
		
		*ps++ = '\0';
		SUBSRCPND = (0x01);   // clear uart0 sub rx interrupt
		printf("uart fifo isr:%s\r\n",buff);
				
	}
	#endif
	
	#if UART_FIFO_SEND_INT	
	int i = 0;
	
	if(SUBSRCPND & (0x01<<1))  //send interrupt
	{
 		putc('S');
		SUBSRCPND = (0x01<<1);   // clear uart0 sub rx interrupt
	}
	#endif
}

void IRQ_Handle()
{
    unsigned long oft = INTOFFSET;
	
	switch( oft )    
	{   
		
		case 0:         
		{               
			EINT0_ISR();
			break;        
		}                
		case 2:        
		{               
			EINT2_ISR();        
			break;        
		}             
		case 5:        
		{              
			EINT8_23_ISR();                     
			break;        
		}     
		case 17:
		{
			DMA0_ISR();
			break;
		}
		case 28:
		{
			UART0_ISR();
			break;
		}
		default:            
			break;   
		}    
	
		SRCPND = 1<<oft;    
		INTPND = 1<<oft;
		
}
