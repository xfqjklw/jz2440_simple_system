#include "include.h"

int main()
{    
	uart0_init();
	led_init();
	key_init();
	//timer0_init();
	//timer1_init();	
	//rtc_init();
	//rtc_alarm_init();
	//rtc_tick_init();

	printf("-------------------------------------------------\r\n");
	printf("                system start %s               \r\n",VERSION);
	printf("-------------------------------------------------\r\n");
	rtc_display();
	
	#if USB_DEVICE_BULK
	usb_device_bulk_init();
	usb_device_bulk_process();
	#endif

	#if USB_DEVICE_MOUSE
	usb_device_mouse_init();
	#endif

	int i = 0;
 	while(1)
	{	
		UsbBulkEp1Handler();
		for(i=0;i<1000000;i++);
 	}
	return 0;
}
