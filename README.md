# jz2440_simple_system
JZ2440 hardware code,use no os,and debug using jtag

Notice:
1.if you want to use openjtag to debug,modify head.S
#define OPENJTAG_DEBUG 1
if you want to run the program from nandflash.
#define OPENJTAG_DEBUG 0

2.if you encounter "GDB missing ack(2)"error when using openjtag to debug.
please add the below in your gdb init file.

set remote memory-write-packet-size 1024
set remote memory-write-packet-size fixed
set remote memory-read-packet-size 1024
set remote memory-read-packet-size fixed

3.cpu frequencyset set 400MHZ  // 
