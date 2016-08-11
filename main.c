#include "include.h"

int main()
{    
	uart0_init();
	led_init();
	key_init();

	#if TIMER_TEST
	timer0_init();
	timer1_init();	
	#endif

	#if RTC_TEST
	rtc_init();
	rtc_alarm_init();
	rtc_tick_init();
	#endif

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
	
	#if WM8976_TEST
	wm8976_init();
	music_play();
	#endif

	//Test_Lcd_Tft_8bit_480272();
	Test_Lcd_Tft_16Bit_480272();

	tc_test();
 	while(1)
	{	
	
 	}
	return 0;
}
