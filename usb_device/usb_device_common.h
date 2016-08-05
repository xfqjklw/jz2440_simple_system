#ifndef _USB_DEVICE_H_
#define _USB_DEVICE_H_

#define U8 unsigned char

/*usb debug*/
#define DEBUG_USB 1

#if DEBUG_USB
#define DbgPrintf(args...) do {\
    printf(args);\
}while(0)
#else
#define DbgPrintf(args...)
#endif

#define REQ_TYPE_MASK       (3<<5)
#define STANDARD_REQ    0
#define CLASS_REQ       (1)

#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO			1
#define USB_CLASS_COMM			2
#define USB_CLASS_HID			3
#define USB_CLASS_PHYSICAL		5
#define USB_CLASS_STILL_IMAGE		6
#define USB_CLASS_PRINTER		7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB			9
#define USB_CLASS_CDC_DATA		0x0a
#define USB_CLASS_CSCID			0x0b	/* chip+ smart card */
#define USB_CLASS_CONTENT_SEC		0x0d	/* content security */
#define USB_CLASS_VIDEO			0x0e
#define USB_CLASS_WIRELESS_CONTROLLER	0xe0
#define USB_CLASS_MISC			0xef
#define USB_CLASS_APP_SPEC		0xfe
#define USB_CLASS_VENDOR_SPEC		0xff


#define CLR_EP0_OUT_PKT_RDY() 		EP0_CSR=( (ep0_csr & (~EP0_WR_BITS))| EP0_SERVICED_OUT_PKT_RDY )	
#define CLR_EP0_OUTPKTRDY_DATAEND()	EP0_CSR=( (ep0_csr & (~EP0_WR_BITS))|(EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END) )	
#define SET_EP0_IN_PKT_RDY() 		EP0_CSR=( (ep0_csr & (~EP0_WR_BITS))|(EP0_IN_PKT_READY) )	 
#define SET_EP0_INPKTRDY_DATAEND() 	EP0_CSR=( (ep0_csr & (~EP0_WR_BITS))|(EP0_IN_PKT_READY|EP0_DATA_END) )
#define CLR_EP0_SETUP_END() 		EP0_CSR=( (ep0_csr & (~EP0_WR_BITS))|(EP0_SERVICED_SETUP_END) )
#define CLR_EP0_SENT_STALL() 		EP0_CSR=( ep0_csr & (~EP0_WR_BITS)& (~EP0_SENT_STALL) )

#define CLR_EP3_OUT_PKT_READY() 	OUT_CSR1_REG= (out_csr3 &(~ EPO_WR_BITS) &(~EPO_OUT_PKT_READY) ) 
#define CLR_EP3_SENT_STALL()		OUT_CSR1_REG= (out_csr3 & (~EPO_WR_BITS) &(~EPO_SENT_STALL) )

#define SET_EP1_IN_PKT_READY()  	IN_CSR1_REG= (( in_csr1 &(~ EPI_WR_BITS))| EPI_IN_PKT_READY )	

/*ep0 state*/
#define EP0_STATE_INIT 			(1)

#define EP0_STATE_GD_DEV_0	 	(10)   
#define EP0_STATE_GD_DEV_1 		(11)
#define EP0_STATE_GD_DEV_2 		(12)

#define EP0_STATE_GD_CFG_0	 	(20)
#define EP0_STATE_GD_CFG_1 		(21)
#define EP0_STATE_GD_CFG_2 		(22)
#define EP0_STATE_GD_CFG_3 		(23)
#define EP0_STATE_GD_CFG_4 		(24)

#define EP0_STATE_GD_STR_I0	 	(30)  
#define EP0_STATE_GD_STR_I1	 	(31)  
#define EP0_STATE_GD_STR_I2	 	(32)  
#define EP0_STATE_GD_STR_I3	 	(33)  

#define EP0_STATE_GD_CFG_ONLY_0		(40)
#define EP0_STATE_GD_CFG_ONLY_1		(41)

#define EP0_HID_REPORT_DES_STR   (50)

//USB_INT_REG / USB_INT_EN_REG
#define SUSPEND_INT            	 0x01  
#define RESUME_INT               0x02  
#define RESET_INT                0x04  

//EP_INT_REG / EP_INT_EN_REG
#define EP0_INT                	 0x01  // Endpoint 0, Control   
#define EP1_INT                  0x02  // Endpoint 1, 
#define EP2_INT                  0x04  // Endpoint 2 
#define EP3_INT			 		 0x08  // Endpoint 3,   
#define EP4_INT			 		 0x10  // Endpoint 4

#define EP0_SENT_STALL           0x04  /* USB sets */       
#define EP0_DATA_END             0x08  /* MCU sets */
#define EP0_SETUP_END            0x10  /* USB sets, MCU clears by setting SERVICED_SETUP_END */
#define EP0_SEND_STALL           0x20  /* MCU sets */
#define EP0_SERVICED_OUT_PKT_RDY 0x40  /* MCU writes 1 to clear OUT_PKT_READY */
#define EP0_SERVICED_SETUP_END   0x80  /* MCU writes 1 to clear SETUP_END        */

#define EP0_WR_BITS              0xc0 // clear SET_UP clear OUT_PKT_RDY

#define EP0_OUT_PKT_READY        0x01  /* USB sets, MCU clears by setting SERVICED_OUT_PKT_RDY */
#define EP0_IN_PKT_READY         0x02  /* MCU sets, USB clears after sending FIFO */

//IN_CSR1
#define EPI_IN_PKT_READY         0x01  
#define EPI_UNDER_RUN		 0x04
#define EPI_FIFO_FLUSH		 0x08
#define EPI_SEND_STALL           0x10  
#define EPI_SENT_STALL           0x20  
#define EPI_CDT			 0x40	
#define EPI_WR_BITS              (EPI_FIFO_FLUSH|EPI_IN_PKT_READY|EPI_CDT) 
					//(EPI_FIFO_FLUSH) is preferred  (???)
//IN_CSR2
#define EPI_IN_DMA_INT_MASK	(1<<4)
#define EPI_MODE_IN		(1<<5)
#define EPI_MODE_OUT		(0<<5)
#define EPI_ISO			(1<<6)
#define EPI_BULK		(0<<6)
#define EPI_AUTO_SET		(1<<7)

//OUT_CSR1
#define EPO_OUT_PKT_READY        0x01  
#define EPO_OVER_RUN		 0x04  
#define EPO_DATA_ERROR		 0x08  
#define EPO_FIFO_FLUSH		 0x10
#define EPO_SEND_STALL           0x20  
#define EPO_SENT_STALL           0x40
#define EPO_CDT			 0x80	
#define EPO_WR_BITS              (EPO_FIFO_FLUSH|EPO_SEND_STALL|EPO_CDT)

//OUT_CSR2
#define EPO_OUT_DMA_INT_MASK	(1<<5)
#define EPO_ISO		 	(1<<6)
#define EPO_BULK	 	(0<<6)
#define EPO_AUTO_CLR		(1<<7)

//USB DMA control register
#define UDMA_IN_RUN_OB		(1<<7)
#define UDMA_IGNORE_TTC		(1<<7)
#define UDMA_DEMAND_MODE	(1<<3)
#define UDMA_OUT_RUN_OB		(1<<2)
#define UDMA_OUT_DMA_RUN	(1<<2)
#define UDMA_IN_DMA_RUN		(1<<1)
#define UDMA_DMA_MODE_EN	(1<<0)

// Standard bmRequestTyje (Direction) 
#define HOST_TO_DEVICE              (0x00)
#define DEVICE_TO_HOST              (0x80)    

// Standard bmRequestType (Type) 
#define STANDARD_TYPE               (0x00)
#define CLASS_TYPE                  (0x20)
#define VENDOR_TYPE                 (0x40)
#define RESERVED_TYPE               (0x60)

// Standard bmRequestType (Recipient) 
#define DEVICE_RECIPIENT            (0)
#define INTERFACE_RECIPIENT         (1)
#define ENDPOINT_RECIPIENT          (2)
#define OTHER_RECIPIENT             (3)

// Feature Selectors 
#define DEVICE_REMOTE_WAKEUP        (1)
#define EP_STALL                    (0)

// Standard Request Codes 
#define GET_STATUS                  (0)
#define CLEAR_FEATURE               (1)
#define SET_FEATURE                 (3)
#define SET_ADDRESS                 (5)
#define GET_DESCRIPTOR              (6)
#define SET_DESCRIPTOR              (7)
#define GET_CONFIGURATION           (8)
#define SET_CONFIGURATION           (9)
#define GET_INTERFACE               (10)
#define SET_INTERFACE               (11)
#define SYNCH_FRAME                 (12)

// Class-specific Request Codes 
#define GET_DEVICE_ID               (0)
#define GET_PORT_STATUS             (1)
#define SOFT_RESET                  (2)

// Descriptor Types
#define DEVICE_TYPE                 (1)
#define CONFIGURATION_TYPE          (2)
#define STRING_TYPE                 (3)
#define INTERFACE_TYPE              (4)
#define ENDPOINT_TYPE               (5)
#define HID_DESCRIPTION_TYPES		0x22  //in hid description type,set 0x22

#define GET_REPORT       0x01
#define GET_IDLE         0x02
#define GET_PROTOCAL     0x03
#define SET_REPORT       0x09
#define SET_IDLE         0x0A
#define SET_PROTOCAL     0x0B

//configuration descriptor: bmAttributes 
#define CONF_ATTR_DEFAULT	    	(0x80) //Spec 1.0 it was BUSPOWERED bit.
#define CONF_ATTR_REMOTE_WAKEUP     (0x20)
#define CONF_ATTR_SELFPOWERED       (0x40)

//endpoint descriptor
#define EP_ADDR_IN		    (0x80)	
#define EP_ADDR_OUT		    (0x00)

#define EP_ATTR_CONTROL		    (0x0)	
#define EP_ATTR_ISOCHRONOUS	    (0x1)
#define EP_ATTR_BULK		    (0x2)
#define EP_ATTR_INTERRUPT	    (0x3)	


//string descriptor
#define LANGID_US_L 		    (0x09)  
#define LANGID_US_H 		    (0x04)


struct USB_SETUP_DATA{
    U8 bmRequestType;    
    U8 bRequest;         
    U8 bValueL;          
    U8 bValueH;          
    U8 bIndexL;          
    U8 bIndexH;          
    U8 bLengthL;         
    U8 bLengthH;         
};


struct USB_DEVICE_DESCRIPTOR{
    U8 bLength;    
    U8 bDescriptorType;         
    U8 bcdUSBL;
    U8 bcdUSBH;
    U8 bDeviceClass;          
    U8 bDeviceSubClass;          
    U8 bDeviceProtocol;          
    U8 bMaxPacketSize0;         
    U8 idVendorL;
    U8 idVendorH;
    U8 idProductL;
    U8 idProductH;
    U8 bcdDeviceL;
    U8 bcdDeviceH;
    U8 iManufacturer;
    U8 iProduct;
    U8 iSerialNumber;
    U8 bNumConfigurations;
};


struct USB_CONFIGURATION_DESCRIPTOR{
    U8 bLength;    
    U8 bDescriptorType;         
    U8 wTotalLengthL;
    U8 wTotalLengthH;
    U8 bNumInterfaces;
    U8 bConfigurationValue;
    U8 iConfiguration;
    U8 bmAttributes;
    U8 maxPower;          
};
    

struct USB_INTERFACE_DESCRIPTOR{
    U8 bLength;    
    U8 bDescriptorType;         
    U8 bInterfaceNumber;
    U8 bAlternateSetting;
    U8 bNumEndpoints;
    U8 bInterfaceClass;
    U8 bInterfaceSubClass;
    U8 bInterfaceProtocol;
    U8 iInterface;
};


struct USB_ENDPOINT_DESCRIPTOR{
    U8 bLength;    
    U8 bDescriptorType;         
    U8 bEndpointAddress;
    U8 bmAttributes;
    U8 wMaxPacketSizeL;
    U8 wMaxPacketSizeH;
    U8 bInterval;
};

 struct USB_CONFIGURATION_SET{
     U8 ConfigurationValue;
 };

 struct USB_GET_STATUS{
     U8 Device;
     U8 Interface;
     U8 Endpoint0;
     U8 Endpoint1;
     U8 Endpoint3;
 };

 struct USB_INTERFACE_GET{
     U8 AlternateSetting;
 };

 struct USB_HID_DESCRIPTOR{
	 U8  bLength;
	 U8  bDescriptorType;
	 U8  bcdHIDL;	 
	 U8  bcdHIDH;
	 U8  bCountryCode;
	 U8  bNumDescriptors;
	 U8  bDescriptorType0;
	 U8  bDescriptorLengthL;	 
	 U8  bDescriptorLengthH;
 };
 
/*usb description*/
extern struct USB_SETUP_DATA descSetup;
extern struct USB_DEVICE_DESCRIPTOR descDev;
extern struct USB_CONFIGURATION_DESCRIPTOR descConf;
extern struct USB_INTERFACE_DESCRIPTOR descIf;
//struct USB_ENDPOINT_DESCRIPTOR descEndpt0;
extern struct USB_ENDPOINT_DESCRIPTOR descEndpt3;
extern struct USB_CONFIGURATION_SET ConfigSet;
extern struct USB_INTERFACE_GET InterfaceGet;
extern struct USB_GET_STATUS StatusGet;


/*function*/
extern void usb_clk_init();
extern void usb_dev_enable();
extern void usb_dev_port_init();

extern void WrPktEp0(U8 *buf,int num);
extern void RdPktEp0(U8 *buf,int num);
extern void WrPktEp1(U8 *buf,int num);
extern void RdPktEp3(U8 *buf,int num);

extern void ConfigEp3IntMode();
extern void ClearEp3OutPktReady();
extern void ConfigEp3DmaMode(unsigned int bufAddr,unsigned int count);

#endif
