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
		GPFDAT |= (0x7<<4); //close all led            
		GPFDAT &= ~(1<<6);  // open led3     	
		//clear interrupt
		EINTPEND = (1<<11);   // EINT8_23ºÏÓÃIRQ5	
	}
}

#if UART_SEND_DMA
extern int uartSendDmaEnd;
#endif

void DMA0_ISR()
{
	#if UART_SEND_DMA
	uartSendDmaEnd = 1;
	printf("uart dma send end\r\n");
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
		default:            
			break;   
		}    
	
		SRCPND = 1<<oft;    
		INTPND = 1<<oft;
		
}
