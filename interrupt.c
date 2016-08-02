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


void TIMER0_ISR()
{
	printf("timer0 isr occur\r\n");


}

void TIMER1_ISR()
{
	printf("timer1 isr occur\r\n");

}

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

void USBD_ISR()
{
	isr_usbd();
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
	
}

void RTC_ISR()
{
	printf("alarm occur\r\n");
}

void TICK_ISR()
{
	printf("tick occur\r\n");
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
		case 8:
		{
			TICK_ISR();
			break;
		}
		case 10:
		{
			TIMER0_ISR();
			break;
		}
		case 11:
		{
			TIMER1_ISR();
			break;
		}	
		case 17:
		{
			DMA0_ISR();
			break;
		}
		case 25:
		{
			USBD_ISR();
			break;
		}
		case 28:
		{
			UART0_ISR();
			break;
		}
		case 30:
		{
			RTC_ISR();
		}
		default:            
			break;   
		}    
	
		SRCPND = 1<<oft;
		INTPND = 1<<oft;
		
}
