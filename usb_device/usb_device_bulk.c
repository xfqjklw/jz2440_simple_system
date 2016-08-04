#include "include.h"
#include "s3c24xx.h"
#include "usb_device_common.h"

#define EP1_PKT_SIZE 64
#define EP3_PKT_SIZE 64
#define EP0_PKT_SIZE 8	
#define DMA_MAX_SIZE 128

static const U8 descStr0[]={
    4,STRING_TYPE,LANGID_US_L,LANGID_US_H,  //codes representing languages
};

static const U8 descStr1[]={  //Manufacturer  
    (0x14+2),STRING_TYPE, 
    'S',0x0,'y',0x0,'s',0x0,'t',0x0,'e',0x0,'m',0x0,' ',0x0,'M',0x0,
    'C',0x0,'U',0x0,
};

static const U8 descStr2[]={  //Product  
    (0x2a+2),STRING_TYPE, 
    'S',0x0,'E',0x0,'C',0x0,' ',0x0,'S',0x0,'3',0x0,'C',0x0,'2',0x0,
    '4',0x0,'1',0x0,'0',0x0,'X',0x0,' ',0x0,'T',0x0,'e',0x0,'s',0x0,
    't',0x0,' ',0x0,'B',0x0,'/',0x0,'D',0x0
};

/*usb description*/
struct USB_SETUP_DATA descSetup;
struct USB_DEVICE_DESCRIPTOR descDev;
struct USB_CONFIGURATION_DESCRIPTOR descConf;
struct USB_INTERFACE_DESCRIPTOR descIf;
struct USB_ENDPOINT_DESCRIPTOR descEndpt1;
struct USB_ENDPOINT_DESCRIPTOR descEndpt3;
struct USB_CONFIGURATION_SET ConfigSet;
struct USB_INTERFACE_GET InterfaceGet;
struct USB_GET_STATUS StatusGet;

volatile int isUsbdSetConfiguration = 0;
volatile unsigned int downloadFileSize = 0;
volatile unsigned int downloadAddress = 0;
volatile unsigned int downloadTotalSize = 0;

U8 *downPt;
int checkSum;
void (*run)(void);

U8 ep1Buf[12] = "123456789012";

void RdPktEp3_CheckSum(U8 *buf,int num)
{
    int i;
	
	
    for(i=0;i<num;i++)
    {
        buf[i]=(U8)EP3_FIFO;
        checkSum+=buf[i];
    }
}

void usb_dev_bulk_config()
{
    PWR_REG = 0;			//SUSPEND_EN = 0

	/*ep0*/
	INDEX_REG = 0;			//choose ep0
    MAXP_REG = 0x01;		// MAXP = 8bytes
    EP0_CSR = (1<<6)|(1<<7);// clear OUT_PKT_RDY,clear SETUP_END
	
    /* ep1 */
    INDEX_REG = 1;
    MAXP_REG = 0x08;					// MAXP = 64bytes
    IN_CSR1_REG = (1<<3);//| (1<<6);		//FIFO_FLUSH =1,CLR_DATA_TOGGLE = 1
    IN_CSR2_REG = (1<<5)|(1<<4)|(0<<6);	//MODE_IN = IN,IN_PKT_RDY interrupt is disable in dma mode,bulk
    OUT_CSR1_REG = (1<<7);				// CLR_DATA_TOGGLE = 1
    OUT_CSR2_REG = (0<<6)|(1<<5);		// in dma mode out_pkt_rdy interrupt disable,bulk
	
	/* ep3 */
    INDEX_REG = 3;	
    MAXP_REG = 0x08;       			// MAXP = 64bytes
    IN_CSR1_REG = (1<<3);//|(1<<6);  	//flush the packet in Input-related FIFO.   PID in packet will maintain DATA0
    IN_CSR2_REG = (0<<5)|(1<<4);	//Configures Endpoint Direction as OUT  IN_DMA interrupt disable
    //OUT_CSR1_REG = (1<<7);		//the data toggle sequence bit is reset to DATA0
    OUT_CSR2_REG = (0<<6)|(1<<5);	//bulk mode,in dma mode out_pkt_rdy interrupt disable
	
	/*interrupt*/
	/*
	*The USB core has two interrupt registers(EP_INT_REG/USB_INT_REG). These registers act as status registers for the MCU when it is
	*interrupted. The bits are cleared by writing a 1 to each bit that was set.
	*/
	EP_INT_REG = (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4);  //clear endpoint interrupt status register EP0~EP4
    USB_INT_REG = (1<<0)|(1<<1)|(1<<2);  			  //clear usb interrupt status register reset/resume/suspend
    EP_INT_EN_REG = (1<<0)|(1<<1)|(1<<3);			  //open endpoint 0/1/3interrupt
    USB_INT_EN_REG = (1<<2);						  //open reset interrupt
    
    INTMSK &= ~(0x01<<25);  //open usb device interrupt
}

void usb_dev_bulk_desc_table_init()
{
    //Standard device descriptor
	descDev.bLength=0x12;					//18 bytes
    descDev.bDescriptorType=DEVICE_TYPE;
    descDev.bcdUSBL=0x10;
    descDev.bcdUSBH=0x01; 					//Ver 1.10
    descDev.bDeviceClass=0xFF; 				//0xff:factory defined
    descDev.bDeviceSubClass=0x0;            
    descDev.bDeviceProtocol=0x0;          
    descDev.bMaxPacketSize0=0x8;         	//package max size 8bytes
    descDev.idVendorL=0x45;
    descDev.idVendorH=0x53;
    descDev.idProductL=0x34;
    descDev.idProductH=0x12;
    descDev.bcdDeviceL=0x00;				//version 1.0
    descDev.bcdDeviceH=0x01;
    descDev.iManufacturer=0x1;  			//index of string descriptor
    descDev.iProduct=0x2;					//index of string descriptor
    descDev.iSerialNumber=0x0;
    descDev.bNumConfigurations=0x1;			//configuration numbers

	//Standard configuration descriptor
    descConf.bLength=0x9;
    descConf.bDescriptorType=CONFIGURATION_TYPE;
    descConf.wTotalLengthL=0x20; 			//<cfg desc>+<if desc>+<endp1 desc>+<endp3 desc>
    descConf.wTotalLengthH=0;
    descConf.bNumInterfaces=1;				//interface numbers
    descConf.bConfigurationValue=1;			//configuration index value
    descConf.iConfiguration=0;              //configuration string index 0 means null
    descConf.bmAttributes=CONF_ATTR_DEFAULT|CONF_ATTR_SELFPOWERED;  //bus powered only.
    descConf.maxPower=25; 					//draws 50mA current from the USB bus.

	//Standard interface descriptor
    descIf.bLength=0x9;
    descIf.bDescriptorType=INTERFACE_TYPE;
    descIf.bInterfaceNumber=0x0;			//interface 0
    descIf.bAlternateSetting=0x0; 
    descIf.bNumEndpoints=2;					//# of endpoints except EP0
    descIf.bInterfaceClass=0xff;			
    descIf.bInterfaceSubClass=0x0;
    descIf.bInterfaceProtocol=0x0;
    descIf.iInterface=0x0;					//interface string index 0 means null

	//Standard endpoint1 descriptor
	descEndpt1.bLength=0x7;    
	descEndpt1.bDescriptorType=ENDPOINT_TYPE;		  
	descEndpt1.bEndpointAddress=1|EP_ADDR_IN;	// 2400Xendpoint 1 is IN endpoint.
	descEndpt1.bmAttributes=EP_ATTR_BULK;
	descEndpt1.wMaxPacketSizeL=EP1_PKT_SIZE; //64
	descEndpt1.wMaxPacketSizeH=0x0;
	descEndpt1.bInterval=0x0; //not used
	
	//Standard endpoint3 descriptor
    descEndpt3.bLength=0x7;
    descEndpt3.bDescriptorType=ENDPOINT_TYPE;         
    descEndpt3.bEndpointAddress=3|EP_ADDR_OUT;   // 2400X endpoint 3 is OUT endpoint.
    descEndpt3.bmAttributes=EP_ATTR_BULK;
    descEndpt3.wMaxPacketSizeL=EP3_PKT_SIZE; //64
    descEndpt3.wMaxPacketSizeH=0x0;
    descEndpt3.bInterval=0x0; //not used 

}



void Ep0Handler()
{	
    static int ep0State = EP0_STATE_INIT;
    static int ep0SubState;	
	
    U8 ep0_csr;
	
	INDEX_REG = 0;
	ep0_csr = EP0_CSR;  //END POINT0 CONTROL STATUS REGISTER (EP0_CSR)

    //DbgPrintf("[ep0 handler, ep0 csr : %x]",ep0_csr);

    if(ep0_csr & EP0_SETUP_END)  //Set by the USB when a control transfer ends before DATA_END is set.
    {	
        DbgPrintf("[SETUPEND]");
		CLR_EP0_SETUP_END();
        if(ep0_csr & EP0_OUT_PKT_READY) 
        {
            /*
			*Set by the USB when a control transfer ends before DATA_END is set.
			*When the USB sets this bit, an interrupt is generated to the MCU.
			*When such a condition occurs, the USB flushes the FIFO and invalidates MCU access to the FIFO.
			*/
			CLR_EP0_OUT_PKT_RDY();
        }
        ep0State=EP0_STATE_INIT;
        return;
	}

    if(ep0_csr & EP0_SENT_STALL)
    {
		DbgPrintf("[STALL]");
		CLR_EP0_SENT_STALL();
		if(ep0_csr & EP0_OUT_PKT_READY) 
		{
			CLR_EP0_OUT_PKT_RDY();
		}

		ep0State=EP0_STATE_INIT;
		return;
	}

	//Set by the USB once a valid token is written to the FIFO
	if((ep0_csr & EP0_OUT_PKT_READY) && (ep0State == EP0_STATE_INIT))
	{
		//DbgPrintf("[EP0_OUT_PKT_READY]");
        RdPktEp0((U8 *)&descSetup,EP0_PKT_SIZE);
		
        switch(descSetup.bRequest)
        {
			case GET_DESCRIPTOR:
			{
				switch(descSetup.bValueH)
				{
					case DEVICE_TYPE:
					{	
						DbgPrintf("[DT]");
						CLR_EP0_OUT_PKT_RDY();  //write a "1" to this bit to clear OUT_PKT_RDY
						ep0State = EP0_STATE_GD_DEV_0;			
						break;
					}
					
					case CONFIGURATION_TYPE:
					{						
						CLR_EP0_OUT_PKT_RDY();  //write a "1" to this bit to clear OUT_PKT_RDY
						if((descSetup.bLengthL+(descSetup.bLengthH<<8))>0x9)
						{
							ep0State=EP0_STATE_GD_CFG_0; 	//win10							
							DbgPrintf("[CT0]");
						}
						else
						{
							ep0State=EP0_STATE_GD_CFG_ONLY_0; //ubuntu12.04							
							DbgPrintf("[CT1]");
						}
						break;
					}

					case STRING_TYPE:
					{
	                	DbgPrintf("[STRING_TYPE]");
	                	CLR_EP0_OUT_PKT_RDY();
		                switch(descSetup.bValueL)
		                {
			                case 0:
			                    ep0State = EP0_STATE_GD_STR_I0;
			                    break;
			                case 1:
			                    ep0State = EP0_STATE_GD_STR_I1;
			                    break;
			                case 2:	
			                    ep0State=EP0_STATE_GD_STR_I2;
			                    break;
			                default:
			                    DbgPrintf("[UE:STRI?%d]",descSetup.bValueL);
			                    break;
		                }
	               	 	ep0SubState=0;
	                	break;
					}
				}
				break;
			}
			
			case SET_ADDRESS:
			{
				DbgPrintf("[SAD:%d]",descSetup.bValueL);
				FUNC_ADDR_REG=descSetup.bValueL | 0x80;  //bit 7 must set 1
    			CLR_EP0_OUTPKTRDY_DATAEND(); //Because of no data control transfers.
    			ep0State=EP0_STATE_INIT;
				break;
			}
			
			case SET_CONFIGURATION:
			{
	            DbgPrintf("[SC]");
	            ConfigSet.ConfigurationValue=descSetup.bValueL;
	            CLR_EP0_OUTPKTRDY_DATAEND(); //Because of no data control transfers.
	            ep0State=EP0_STATE_INIT;
				isUsbdSetConfiguration = 1;
				break;
			}
						
		}
	}

	switch(ep0State)
	{
		case EP0_STATE_INIT:
       		break;
		
        //=== GET_DESCRIPTOR:DEVICE ===
		case EP0_STATE_GD_DEV_0:
		{
			//DbgPrintf("[GDD0]");
			WrPktEp0((U8 *)&descDev+0,8); 	//EP0_PKT_SIZE
			SET_EP0_IN_PKT_RDY();		  	//reply usb device description.
			//Set by the MCU after writing a packet of data into EP0 FIFO. 
			//An interrupt is generated when the USB clears this bit, so as the MCU to load the next packet. 
			ep0State = EP0_STATE_GD_DEV_1;
			break;
		}
		
		case EP0_STATE_GD_DEV_1:
		{
			//DbgPrintf("[GDD1]");	     	//don't add printf here
			WrPktEp0((U8 *)&descDev+0x8,8);
			SET_EP0_IN_PKT_RDY();
        	ep0State=EP0_STATE_GD_DEV_2;
			break;
		}
		
		case EP0_STATE_GD_DEV_2:
		{	
			//DbgPrintf("[GDD2]");
	        WrPktEp0((U8 *)&descDev+0x10,2);   //8+8+2=0x12
	        SET_EP0_INPKTRDY_DATAEND();			//zero length data packet data end
	        ep0State=EP0_STATE_INIT;			
			break;
		}

        //=== GET_DESCRIPTOR:CONFIGURATION+INTERFACE+ENDPOINT3 ===
		case EP0_STATE_GD_CFG_0:
		{	
			//DbgPrintf("[GDC0]");
			WrPktEp0((U8 *)&descConf+0,8); //EP0_PKT_SIZE
       	 	SET_EP0_IN_PKT_RDY();			
			ep0State=EP0_STATE_GD_CFG_1;			
			break;
		}
		
		case EP0_STATE_GD_CFG_1:
		{
			//DbgPrintf("[GDC1]");
	        WrPktEp0((U8 *)&descConf+8,1); 
	        WrPktEp0((U8 *)&descIf+0,7); 
	        SET_EP0_IN_PKT_RDY();
	        ep0State=EP0_STATE_GD_CFG_2;
	        break;	
		}

		case EP0_STATE_GD_CFG_2:
		{
			//DbgPrintf("[GDC2]");
			WrPktEp0((U8 *)&descIf+7,2); 
			WrPktEp0((U8 *)&descEndpt1+0,6); 
			SET_EP0_IN_PKT_RDY();
			ep0State=EP0_STATE_GD_CFG_3;
			break;
		}
		
		case EP0_STATE_GD_CFG_3:
		{
	        //DbgPrintf("[GDC3]");
	        WrPktEp0((U8 *)&descEndpt1+1,1);			
	        WrPktEp0((U8 *)&descEndpt3+0,7);			
			SET_EP0_IN_PKT_RDY(); //zero length data packet data end 
 	        ep0State=EP0_STATE_GD_CFG_4;            
	        break;
		}

		case EP0_STATE_GD_CFG_4:
		{
			//DbgPrintf("[GDC4]");		
	        //zero length data packet 
	        SET_EP0_INPKTRDY_DATAEND();
	        ep0State=EP0_STATE_INIT;            
	        break;
		}
      
        //=== GET_DESCRIPTOR:STRING ===
		case EP0_STATE_GD_STR_I0:
		{	
			//DbgPrintf("[GDS0_0]");
	        WrPktEp0((U8 *)descStr0, 4 );  
	        SET_EP0_INPKTRDY_DATAEND();
	        ep0State=EP0_STATE_INIT;     
	        ep0SubState=0;
			break;
		}
		
		case EP0_STATE_GD_STR_I1:
		{
			//DbgPrintf("[GDS1_%d]",ep0SubState);
	        if( (ep0SubState*EP0_PKT_SIZE+EP0_PKT_SIZE)<sizeof(descStr1) )
	        {
	            WrPktEp0((U8 *)descStr1+(ep0SubState*EP0_PKT_SIZE),EP0_PKT_SIZE); 
	            SET_EP0_IN_PKT_RDY();
	            ep0State=EP0_STATE_GD_STR_I1;
	            ep0SubState++;
	        }
	        else
	        {
	            WrPktEp0((U8 *)descStr1+(ep0SubState*EP0_PKT_SIZE),sizeof(descStr1)-(ep0SubState*EP0_PKT_SIZE)); 
	            SET_EP0_INPKTRDY_DATAEND();
	            ep0State=EP0_STATE_INIT;     
	            ep0SubState=0;
	        }
	        break;
		}

		case EP0_STATE_GD_STR_I2:
		{
			//DbgPrintf("[GDS2_%d]",ep0SubState);
	        if( (ep0SubState*EP0_PKT_SIZE+EP0_PKT_SIZE)<sizeof(descStr2) )
	        {
	            WrPktEp0((U8 *)descStr2+(ep0SubState*EP0_PKT_SIZE),EP0_PKT_SIZE); 
	            SET_EP0_IN_PKT_RDY();
	            ep0State=EP0_STATE_GD_STR_I2;
	            ep0SubState++;
	        }
	        else
	        {
	            //DbgPrintf("[E]");
	            WrPktEp0((U8 *)descStr2+(ep0SubState*EP0_PKT_SIZE),sizeof(descStr2)-(ep0SubState*EP0_PKT_SIZE)); 
	            SET_EP0_INPKTRDY_DATAEND();
	            ep0State=EP0_STATE_INIT;     
	            ep0SubState=0;
	        }
	        break;
		}

		//=== GET_DESCRIPTOR:CONFIGURATION ONLY===
    	case EP0_STATE_GD_CFG_ONLY_0:
		{
	        //DbgPrintf("[GDCO0]");
	        WrPktEp0((U8 *)&descConf+0,8); //EP0_PKT_SIZE
	        SET_EP0_IN_PKT_RDY();
	        ep0State=EP0_STATE_GD_CFG_ONLY_1;
        	break;
    	}
		
    	case EP0_STATE_GD_CFG_ONLY_1:
		{
	        //DbgPrintf("[GDCO1]");
	        WrPktEp0((U8 *)&descConf+8,1);
	        SET_EP0_INPKTRDY_DATAEND();
	        ep0State=EP0_STATE_INIT;            
        	break;
    	}	
	}
}

void Ep1Handler()
{
    U8 in_csr1;

    INDEX_REG=1;
    in_csr1 = IN_CSR1_REG;

    DbgPrintf("<1:%x]",in_csr1);
	
    WrPktEp1(ep1Buf,12);
    SET_EP1_IN_PKT_READY(); 
		
}

void Ep3Handler()
{
    U8 out_csr3;
    int fifoCnt;
	U8 tmpRecv[8];
	
    INDEX_REG = 3;
	out_csr3 = OUT_CSR1_REG;	
	
    if(out_csr3 & EPO_OUT_PKT_READY)
    {
        fifoCnt = OUT_FIFO_CNT1_REG;
		printf("EP3 fifoCnt=%d\r\n",fifoCnt);
		
        if(downloadFileSize == 0)
        {
        	downloadTotalSize = 0;
			downloadTotalSize += fifoCnt;
            RdPktEp3((U8 *)tmpRecv,8);
			downloadAddress = *((U8 *)(tmpRecv+0))+(*((U8 *)(tmpRecv+1))<<8)+(*((U8 *)(tmpRecv+2))<<16)+(*((U8 *)(tmpRecv+3))<<24);
			printf("downloadAddress=0x%x\r\n",downloadAddress);			
			downPt = (U8 *)downloadAddress;
			downloadFileSize = *((U8 *)(tmpRecv+4))+(*((U8 *)(tmpRecv+5))<<8)+(*((U8 *)(tmpRecv+6))<<16)+(*((U8 *)(tmpRecv+7))<<24);
			printf("downloadFileSize=%d\r\n",downloadFileSize);
			checkSum = 0;
			RdPktEp3_CheckSum((U8 *)downPt,fifoCnt-8); //The first 8-bytes are deleted.	
			downPt+=fifoCnt-8;

			#if USB_DEVICE_BULK_OUT_DMA
			INTMSK|=(0x01<<25); //close usb device interrupt useful? not sure
            return;	
			#endif
		}
		else
		{	
			//no dma mode		
			RdPktEp3_CheckSum((U8 *)downPt,fifoCnt);	    
            downPt+=fifoCnt;  //fifoCnt=64            
			downloadTotalSize += fifoCnt;
		}
		
		CLR_EP3_OUT_PKT_READY();
	}

	if(out_csr3 & EPO_SENT_STALL)
    {   
        DbgPrintf("[STALL]");
        CLR_EP3_SENT_STALL();
        return;
    }	
}

void isr_usbd()
{	
    U8 usbdIntpnd,epIntpnd;
    U8 saveIndexReg = INDEX_REG;
	
	usbdIntpnd = USB_INT_REG;
    epIntpnd = EP_INT_REG;
	
    if(usbdIntpnd & SUSPEND_INT)
    {
		DbgPrintf( "[SUS]");
		USB_INT_REG = SUSPEND_INT;	//clear interrupt
	}
	
	if(usbdIntpnd & RESUME_INT)
	{
		DbgPrintf("[RSM]");
	   	USB_INT_REG = RESUME_INT; 	//clear interrupt
	}

	if(usbdIntpnd & RESET_INT)
    {
        DbgPrintf( "[RST]");
        usb_dev_bulk_config();
		USB_INT_REG = RESET_INT;	//clear interrupt
    }

	if(epIntpnd & EP0_INT)
    {    	
        //DbgPrintf("[EP0]");
        EP_INT_REG = EP0_INT;  		//clear interrupt
        Ep0Handler();
    }

	if(epIntpnd & EP1_INT)
    {
        DbgPrintf("[EP1]");
        EP_INT_REG = EP1_INT;  
        Ep1Handler();
    }

	if(epIntpnd & EP3_INT)
    {
		//DbgPrintf( "[EP3]");
        EP_INT_REG = EP3_INT;
        Ep3Handler();
    }
	INDEX_REG = saveIndexReg;
}

void ConfigEp3IntMode(void)
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

void ClearEp3OutPktReady(void)
{
    U8 out_csr3;

    INDEX_REG=3;
    out_csr3= OUT_CSR1_REG;
    CLR_EP3_OUT_PKT_READY();
}

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

void isr_dma2()
{
    U8 out_csr3;
	
    U8 saveIndexReg=INDEX_REG;

	printf("isr_dma2\r\n");

    INDEX_REG = 3;
    out_csr3 = OUT_CSR1_REG;
	downloadTotalSize += DMA_MAX_SIZE;// 524288(10) = 0x80000;524288/1024 = 512
	downPt += DMA_MAX_SIZE;
	printf("downloadTotalSize = %d,downPt = 0x%x\r\n",downloadTotalSize,downPt);
	if(downloadTotalSize >= downloadFileSize)
	{
        downloadTotalSize = downloadFileSize;
        ConfigEp3IntMode();	
		if(out_csr3& EPO_OUT_PKT_READY)
        {
            CLR_EP3_OUT_PKT_READY();
        }
		INTMSK|=(0x01<<19); //close dma2 interrupt  
        INTMSK&=~(0x01<<25);//open usb device interrupt 
	}
	else
	{
		if((downloadFileSize - downloadTotalSize) > DMA_MAX_SIZE)
		{
			ConfigEp3DmaMode(downPt,DMA_MAX_SIZE); //config dma
		}
		else
		{
			ConfigEp3DmaMode(downPt,downloadFileSize-downloadTotalSize);
		}
	}
		
    INDEX_REG=saveIndexReg;
}


void usb_device_bulk_init()
{
	usb_clk_init();
	usb_dev_enable();
	usb_dev_bulk_config();	
	usb_dev_port_init();
	usb_dev_bulk_desc_table_init();
	
}

void usb_device_bulk_process()
{	
	int first = 1;
	
	if(isUsbdSetConfiguration == 0)
    {
        printf("USB host is not connected yet.\r\n");
    }
    while(downloadFileSize == 0)
	{
		if(first == 1 && isUsbdSetConfiguration == 1)
		{
			printf("USB host is connected. Waiting a download.\r\n");
			first = 0;
		}	
	}
	
	#if USB_DEVICE_BULK_OUT_DMA
	INTMSK&=~(0x01<<19); 	// enable dma2 interrupt 
    ClearEp3OutPktReady(); 	//clear OUT_PKT_RDY,beacuse data read finished
	
    if(downloadFileSize > EP3_PKT_SIZE)  //if downloadFileSize < EP3_PKT_SIZE don't use dma
    {
        if(downloadFileSize<=DMA_MAX_SIZE) //dma max size is 512k bytes,if downloadFileSize > 512bytes,need do another dma
        {
			printf("downloadFileSize<0x80000\r\n");
            ConfigEp3DmaMode(downPt,downloadFileSize-downloadTotalSize); //config dma
		}
		else
		{
			ConfigEp3DmaMode(downPt,DMA_MAX_SIZE); //config dma
		}
	}
	#endif

	while(1)
	{
		if(downloadTotalSize >= downloadFileSize)
		{
			printf("USB download finished.\r\n");
			INTMSK=0xffffffff;
			printf("run address : 0x%x\r\n", downloadAddress);
			run = (void (*)(void))downloadAddress;
			run();
		}
	}

	
}
