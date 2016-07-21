#include "s3c24xx.h"

void EINT_Handle()
{
    unsigned long oft = INTOFFSET;    
	unsigned long val;        
	switch( oft )    
	{        
		case 0:         
		{               
			GPFDAT |= (0x7<<4); //close all led  
			GPFDAT &= ~(1<<4);  //opem led1    
			break;        
		}                
		case 2:        
		{               
			GPFDAT |= (0x7<<4); //close all led       
			GPFDAT &= ~(1<<5);  //open led2          
			break;        
		}             
		case 5:        
		{              
			GPFDAT |= (0x7<<4); //close all led            
			GPFDAT &= ~(1<<6);  // open led3                          
			break;        
		}        
		default:            
			break;   
		}    
	
		//清中断    
		if( oft == 5 )         
			EINTPEND = (1<<11);   // EINT8_23合用IRQ5   
		SRCPND = 1<<oft;    
		INTPND = 1<<oft;
		
}