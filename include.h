#include "stdio.h"

#define VERSION "v0.1"

/*****uart function******/
#define UART_SEND_DMA 0
#define UART_RECV_DMA 0
#define UART_FIFO_RECV_INT 1  //use recv fifo and interrupt

/*****uart api*****/
extern void uart0_init();

/*****led api*****/
extern void led_init();

/*****key api*****/
extern void key_init();
