#include "s3c24xx.h"
#include "stdio.h"

void rtc_init()
{
	RTCCON = 1;  //RTC control enable
	
	BCDYEAR = 0x16 ;
	BCDMON	= 0x07 ;	 
	BCDDATE = 0x27 ;	  
	BCDDAY	= 0x03 ;	   
	BCDHOUR = 0x17 ;	   
	BCDMIN	= 0x08 ;	   
	BCDSEC	= 0x30 ;	   
	 	 
	RTCCON &= ~1 ;	  //RTC control disable  
	
}

void rtc_display()
{
	int year,month,day,hour,minute,second,week;
	
	
	RTCCON = 1 ;		//RTC read and write enable  

	year = 0x2000+BCDYEAR  ;	   //年  
	month = BCDMON	;	   //月  
	day = BCDDATE  ;	   //日   
	week = BCDDAY  ;	   //星期  
	hour = BCDHOUR	;	   //小时  
	minute = BCDMIN  ;	   //分  
	second = BCDSEC  ;	   //秒  
	 		 
	RTCCON &= ~1 ;	   //RTC read and write disable  

	
	printf( "RTC time : %04x-%02x-%02x %02x:%02x:%02x\r\n", year, month, day, hour, minute, second );	
}

void rtc_alarm_init()
{
	RTCCON = 1;  //RTC control enable
	
	ALMYEAR = 0x16 ;
	ALMMON	= 0x07 ;	 
	ALMDATE = 0x27 ;	  
	ALMHOUR = 0x17 ;	   
	ALMMIN	= 0x09 ;	   
	ALMSEC	= 0x30 ;	   
	 	 
	RTCCON &= ~1 ;	  //RTC control disable 

	RTCALM = 0x7f;   //enable alarm

	INTMSK &= ~(0x01<<30); //enable rtc interrupt
}

void rtc_tick_init()
{
	TICNT = 0xbf; //enable tick interrupt and 1s	
	INTMSK &= ~(0x01<<8); //enable tick interrupt
}
