#include "stdio.h"

#define VERSION "v0.1"

/*uart function*/
#define UART_SEND_DMA 0
#define UART_RECV_DMA 0
#define UART_FIFO_RECV_INT 1  //use recv fifo and interrupt

/*usb function*/
#define USB_DEVICE_BULK  0
#define USB_DEVICE_BULK_OUT_DMA 1

#define USB_DEVICE_MOUSE 1

/*wm8976 function*/
#define WM8976_TEST 	1
#define WM8960_BYPASS_TEST 1

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
extern void usb_bulk_isr_dma2();
extern void usb_bulk_isr_usbd();
extern void usb_device_bulk_init();
extern void usb_device_bulk_process();

extern void usb_device_mouse_init();
extern void usb_mouse_isr_usbd();
extern void usb_mouse_left_press();
extern void usb_mouse_left_release();
extern void usb_mouse_right_release();
extern void usb_mouse_right_release();


