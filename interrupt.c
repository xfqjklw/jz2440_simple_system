#include "include.h"
#include "s3c24xx.h"

void EINT0_ISR()
{
	GPFDAT |= (0x7<<4); //close all led  
	GPFDAT &= ~(1<<6);  //opem led1  

	#if USB_DEVICE_MOUSE
	usb_mouse_left_press();
	#endif
}


void EINT2_ISR()
{
	GPFDAT |= (0x7<<4); //close all led       
	GPFDAT &= ~(1<<5);  //open led2
	
	#if USB_DEVICE_MOUSE
	usb_mouse_right_press();
	#endif
}



void EINT8_23_ISR()
{	
	if(EINTPEND & (1<<11))
	{
		GPFDAT |= (0x7<<4); // close all led            
		GPFDAT &= ~(1<<4);  // open led3     	
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

void DMA2_ISR()
{
	#if USB_DEVICE_BULK
	#if USB_DEVICE_BULK_OUT_DMA
	usb_bulk_isr_dma2();
	#endif
	#endif
}

void USBD_ISR()
{
	#if USB_DEVICE_BULK
	usb_bulk_isr_usbd();
	#endif
	#if USB_DEVICE_MOUSE
	usb_mouse_isr_usbd();
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
	
}

void RTC_ISR()
{
	printf("alarm occur\r\n");
}

void TICK_ISR()
{
	printf("tick occur\r\n");
}

void ADC_ISR()
{	
	printf("adc isr\r\n");
    if(SUBSRCPND & 0x1<<9)  //tc
    {
		Isr_Tc();
		SUBSRCPND |= (0x1<<9);
    }
	if(SUBSRCPND & 0x1<<10) //rtc
	{
		Isr_Adc();
		SUBSRCPND |= (0x1<<10);
	}
	
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
		case 19:
		{
			DMA2_ISR();
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
			break;
		}
		case 31:
		{
			ADC_ISR();
			break;
		}
		default:            
			break;   
		}    
	
		SRCPND = 1<<oft;
		INTPND = 1<<oft;
		
}
