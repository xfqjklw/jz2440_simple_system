#include "include.h"
#include "s3c24xx.h"
#include "usb_device_common.h"

#if USB_DEVICE_MOUSE

#define EP0_PKT_SIZE 8	
#define EP1_PKT_SIZE 64

int left_key_press = 0;
int right_key_press = 0;

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

static const U8 descStr3[] = {
	20,
	STRING_TYPE,
	'U',0x00,'S',0x00,'B',0x00,'M',0x00,'O',0x00,'U',0x00,'S',0x00,'E',0x00,
	' ',0x00,
};

typedef struct {
    unsigned char     ucButton;           /**< Button       */
    unsigned char     ucX;                /**< X            */
    unsigned char     ucY;                /**< Y            */
    unsigned char     ucZ;                /**< Z (Not Used) */
} SampleInputReport_t;

static const SampleInputReport_t l_tSampleInputReport[9] = {
    {0x02U, 0x00U, 0x00U, 0x00U},   /* right key press     */
    {0x00U, 0x00U, 0x00U, 0x00U},   /* right key release   */
    {0x00U, 0xF6U, 0x00U, 0x00U},   /* left move 10 pixel  */
    {0x00U, 0x00U, 0x0AU, 0x00U},   /* down move 10 pixel  */
    {0x00U, 0xF6U, 0x00U, 0x00U},   /* left move 10 pixel  */
    {0x01U, 0x00U, 0x00U, 0x00U},   /* left key press      */
    {0x00U, 0x00U, 0x00U, 0x00U},   /* left key release    */
    {0x00U, 0x00U, 0xF6U, 0x00U},   /* up move 10 pixel    */
    {0x00U, 0x0AU, 0x00U, 0x00U}    /* right move 10 pixel */
};

/*usb description*/
struct USB_SETUP_DATA descSetup;
struct USB_DEVICE_DESCRIPTOR descDev;
struct USB_CONFIGURATION_DESCRIPTOR descConf;
struct USB_INTERFACE_DESCRIPTOR descIf;
struct USB_ENDPOINT_DESCRIPTOR descEndpt1;
struct USB_HID_DESCRIPTOR descHid;
struct USB_CONFIGURATION_SET ConfigSet;
struct USB_INTERFACE_GET InterfaceGet;
struct USB_GET_STATUS StatusGet;

volatile int isUsbdSetConfiguration = 0;

const char gHIDReportDescriptor[] = {
    0x05U, 0x01U,                   /* Usage Page (Generic Desktop)         */
    0x09U, 0x02U,                   /* Usage (Mouse)                        */
    0xA1U, 0x01U,                   /* Collection (Application)             */
    0x09U, 0x01U,                   /*   Usage (Pointer)                    */
    0xA1U, 0x00U,                   /*   Collection (Physical)              */
    0x05U, 0x09U,                   /*     Usage Page (Buttons)             */
    0x19U, 0x01U,                   /*     Usage Minimum (01)               */
    0x29U, 0x03U,                   /*     Usage Maximum (03)               */
    0x15U, 0x00U,                   /*     Logical Minimum (0)              */
    0x25U, 0x01U,                   /*     Logical Maximum (1)              */
    0x95U, 0x03U,                   /*     Report Count (1)                 */
    0x75U, 0x01U,                   /*     Report Size (1)                  */
    0x81U, 0x02U,                   /*     Input (Data, Variable, Absolute) */
    0x95U, 0x01U,                   /*     Report Count (1)                 */
    0x75U, 0x05U,                   /*     Report Size (5)                  */
    0x81U, 0x01U,                   /*     Input (Constant) for padding     */
    0x05U, 0x01U,                   /*     Usage Page (Generic Desktop)     */
    0x09U, 0x30U,                   /*     Usage (X)                        */
    0x09U, 0x31U,                   /*     Usage (Y)                        */
    0x09U, 0x38U,                   /*     Usage (Z)                        */
    0x15U, 0x81U,                   /*     Logical Minimum (-127)           */
    0x25U, 0x7FU,                   /*     Logical Maximum (127)            */
    0x75U, 0x08U,                   /*     Report Size (8)                  */
    0x95U, 0x03U,                   /*     Report Count (2)                 */
    0x81U, 0x06U,                   /*     Input (Data, Variable, Relative) */
    0xC0U,                          /*   End Collection (Physical)          */
    0xC0U                           /* End Collection (Application)         */
};


void UsbMouseEp1Handler()
{
	if(left_key_press == 1)
	{
		usb_mouse_left_release();
	}
	
	if(right_key_press == 1)
	{
		usb_mouse_right_release();
	}
	
}

void UsbMouseEp0Handler()
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
		
		
		if(((descSetup.bmRequestType & REQ_TYPE_MASK)>>5) == CLASS_REQ)
		{			
			printf("[CLASS %d]",descSetup.bRequest);
			switch(descSetup.bRequest)
			{
				case GET_REPORT:
				{
					
					break;
				}
				case SET_IDLE:
				{
					DbgPrintf("[SET_IDLE]");
					CLR_EP0_OUTPKTRDY_DATAEND();
					break;
				}
				
			}
			
		}

		if(((descSetup.bmRequestType & REQ_TYPE_MASK)>>5) == STANDARD_REQ)
		{
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
		                	DbgPrintf("[STRING_TYPE%d]",descSetup.bValueL);
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
				                    ep0State = EP0_STATE_GD_STR_I2;
				                    break;
								case 3:
									ep0State = EP0_STATE_GD_STR_I3;
									break;
				                default:
				                    DbgPrintf("[UE:STRI?%d]",descSetup.bValueL);
				                    break;
			                }
		               	 	ep0SubState = 0;
		                	break;
						}
						
						case HID_DESCRIPTION_TYPES:
						{							
		                	CLR_EP0_OUT_PKT_RDY();
							DbgPrintf("[HID_DES_TYPES]");
							ep0State = EP0_HID_REPORT_DES_STR;
							ep0SubState = 0;
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
			WrPktEp0((U8 *)&descHid+0,6); 
			SET_EP0_IN_PKT_RDY();
			ep0State=EP0_STATE_GD_CFG_3;
			break;
		}
		
		case EP0_STATE_GD_CFG_3:
		{
	        //DbgPrintf("[GDC3]");
	        WrPktEp0((U8 *)&descHid+6,3);			
	        WrPktEp0((U8 *)&descEndpt1+0,5);			
			SET_EP0_IN_PKT_RDY(); //zero length data packet data end 
 	        ep0State=EP0_STATE_GD_CFG_4;            
	        break;
		}

		case EP0_STATE_GD_CFG_4:
		{
			//DbgPrintf("[GDC4]");		
	        //zero length data packet 	        
	        WrPktEp0((U8 *)&descEndpt1+5,2);			
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

		case EP0_STATE_GD_STR_I3:
		{
			//DbgPrintf("[GDS3_%d]",ep0SubState);
	        if( (ep0SubState*EP0_PKT_SIZE+EP0_PKT_SIZE)<sizeof(descStr3) )
	        {
	            WrPktEp0((U8 *)descStr3+(ep0SubState*EP0_PKT_SIZE),EP0_PKT_SIZE); 
	            SET_EP0_IN_PKT_RDY();
	            ep0State=EP0_STATE_GD_STR_I3;
	            ep0SubState++;
	        }
	        else
	        {
	            //DbgPrintf("[E]");
	            WrPktEp0((U8 *)descStr3+(ep0SubState*EP0_PKT_SIZE),sizeof(descStr3)-(ep0SubState*EP0_PKT_SIZE)); 
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

		//=== USB HID REPORT DESCRIPTION===
		case EP0_HID_REPORT_DES_STR:
		{
			if( (ep0SubState*EP0_PKT_SIZE+EP0_PKT_SIZE)<sizeof(gHIDReportDescriptor) )
	        {	        	
	        	//DbgPrintf("[HIDRD_%d]",ep0SubState);
	            WrPktEp0((U8 *)gHIDReportDescriptor+(ep0SubState*EP0_PKT_SIZE),EP0_PKT_SIZE); 
	            SET_EP0_IN_PKT_RDY();
	            ep0State=EP0_HID_REPORT_DES_STR;
	            ep0SubState++;				
	        }
	        else
	        {
	            //DbgPrintf("[E]");
	            WrPktEp0((U8 *)gHIDReportDescriptor+(ep0SubState*EP0_PKT_SIZE),sizeof(gHIDReportDescriptor)-(ep0SubState*EP0_PKT_SIZE)); 
	            SET_EP0_INPKTRDY_DATAEND();
	            ep0State=EP0_STATE_INIT;
	            ep0SubState=0;
	        }
			break;
		}
	}
}


void usb_dev_mouse_config()
{
    PWR_REG = 0;			//SUSPEND_EN = 0

	/*ep0*/
	INDEX_REG = 0;			//choose ep0
    MAXP_REG = 0x01;		// MAXP = 8bytes
    EP0_CSR = (1<<6)|(1<<7);// clear OUT_PKT_RDY,clear SETUP_END
	
    /* ep1 */
    INDEX_REG = 1;
    MAXP_REG = 0x04;					// MAXP = 32bytes
	//default is ok
	//IN_CSR1_REG = (1<<3);//| (1<<6);		//FIFO_FLUSH =1,CLR_DATA_TOGGLE = 1
    //IN_CSR2_REG = (1<<5)|(1<<4)|(1>>6);	//MODE_IN = IN,IN_PKT_RDY interrupt is disable in dma mode
    //OUT_CSR1_REG = (1<<7);				// CLR_DATA_TOGGLE = 1
    //OUT_CSR2_REG = (0<<6)|(1<<5);		// in dma mode out_pkt_rdy interrupt disable,bulk
	
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


void usb_dev_mouse_desc_table_init()
{
    //Standard device descriptor
	descDev.bLength=0x12;					//18 bytes
    descDev.bDescriptorType=DEVICE_TYPE;
    descDev.bcdUSBL=0x10;
    descDev.bcdUSBH=0x01; 					//Ver 1.10
    descDev.bDeviceClass=0x0; 				//
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
    descDev.iSerialNumber=0x3;
    descDev.bNumConfigurations=0x1;			//configuration numbers

	//Standard configuration descriptor
    descConf.bLength=0x9;
    descConf.bDescriptorType=CONFIGURATION_TYPE;
    descConf.wTotalLengthL=0x22; 			//<cfg desc>+<if desc>+<endp1 desc>+<hid desc>
    descConf.wTotalLengthH=0;
    descConf.bNumInterfaces=1;				//interface numbers
    descConf.bConfigurationValue=1;			//configuration index value
    descConf.iConfiguration=0;              //configuration string index 0 means null
    descConf.bmAttributes=CONF_ATTR_DEFAULT|CONF_ATTR_SELFPOWERED;  //bus powered only.
    descConf.maxPower=0x31; 					//draws 50mA current from the USB bus.

	//Standard interface descriptor
    descIf.bLength=0x9;
    descIf.bDescriptorType=INTERFACE_TYPE;
    descIf.bInterfaceNumber=0x0;			//interface 0
    descIf.bAlternateSetting=0x0; 
    descIf.bNumEndpoints=1;					//# of endpoints except EP0
    descIf.bInterfaceClass=USB_CLASS_HID;			
    descIf.bInterfaceSubClass=0x0;
    descIf.bInterfaceProtocol=0x2;
    descIf.iInterface=0x0;					//interface string index 0 means null

	//Hid interface descriptor
	descHid.bLength=0x09;
	descHid.bDescriptorType=0x21;
	descHid.bcdHIDL=0x10;
	descHid.bcdHIDH=0x01;
	descHid.bCountryCode=0x21;
	descHid.bNumDescriptors=0x01;
	descHid.bDescriptorType0=0x22;
	descHid.bDescriptorLengthL=sizeof(gHIDReportDescriptor)&0xFF;
	descHid.bDescriptorLengthH=(sizeof(gHIDReportDescriptor)>>8)&0xFF;
		
	//Standard endpoint1 descriptor
	descEndpt1.bLength=0x7;    
	descEndpt1.bDescriptorType=ENDPOINT_TYPE;		  
	descEndpt1.bEndpointAddress=1|EP_ADDR_IN;	// 2400Xendpoint 1 is IN endpoint.
	descEndpt1.bmAttributes=EP_ATTR_INTERRUPT;
	descEndpt1.wMaxPacketSizeL=EP1_PKT_SIZE; //64
	descEndpt1.wMaxPacketSizeH=0x0;
	descEndpt1.bInterval=0x0A; // 10ms
	
}

void usb_device_mouse_init()
{
	usb_clk_init();
	usb_dev_enable();
	usb_dev_mouse_config();	
	usb_dev_port_init();
	usb_dev_mouse_desc_table_init();
}

void usb_mouse_isr_usbd()
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
        usb_dev_mouse_config();
		USB_INT_REG = RESET_INT;	//clear interrupt
    }

	if(epIntpnd & EP0_INT)
    {    	
        //DbgPrintf("[EP0]");
        EP_INT_REG = EP0_INT;  		//clear interrupt
        UsbMouseEp0Handler();
    }

	if(epIntpnd & EP1_INT)
    {
        //DbgPrintf("[EP1]");
        EP_INT_REG = EP1_INT;
		UsbMouseEp1Handler();
    }

	INDEX_REG = saveIndexReg;
}


void usb_mouse_left_press()
{	   
	DbgPrintf("usb_mouse_left_press\r\n");
	U8 in_csr1;
    INDEX_REG=1;
    in_csr1 = IN_CSR1_REG;
	WrPktEp1(&l_tSampleInputReport[5],4);
	SET_EP1_IN_PKT_READY();
	left_key_press = 1;
}

void usb_mouse_left_release()
{	    
	DbgPrintf("usb_mouse_left_release\r\n");
	U8 in_csr1;
    INDEX_REG=1;
    in_csr1 = IN_CSR1_REG;
	WrPktEp1(&l_tSampleInputReport[6],4);
	SET_EP1_IN_PKT_READY();	
	left_key_press = 0;
}

void usb_mouse_right_press()
{	    
	U8 in_csr1;
    INDEX_REG=1;
    in_csr1 = IN_CSR1_REG;
	WrPktEp1(&l_tSampleInputReport[0],4);
	SET_EP1_IN_PKT_READY();	
	right_key_press = 1;
}

void usb_mouse_right_release()
{	    
	U8 in_csr1;
    INDEX_REG=1;
    in_csr1 = IN_CSR1_REG;
	WrPktEp1(&l_tSampleInputReport[1],4);
	SET_EP1_IN_PKT_READY();	
	right_key_press = 0;
}


#endif
