#include "stdio.h"

#define VERSION "v0.1"

/*uart function*/
#define UART_SEND_DMA 0
#define UART_RECV_DMA 0
#define UART_FIFO_RECV_INT 1  //use recv fifo and interrupt

/*uart api*/
extern void uart0_init();

/*led api*/
extern void led_init();

/*key api*/
extern void key_init();

/*timer api*/
extern void timer0_init();
extern void timer1_init();

/*watchdog*/
extern void disable_watch_dog();

/*usb*/
extern void isr_usbd();
extern void usb_device_bulk_init();

