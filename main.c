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
	
	usb_device_bulk_init();
	usb_device_bulk_process();

 	while(1)
	{
		
 	}
	return 0;
}
