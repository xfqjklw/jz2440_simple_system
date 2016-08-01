objs := head.o nand.o main.o init.o interrupt.o led.o watchdog.o key.o uart.o timer.o rtc.o printf/libc.a

simple.bin: $(objs)
	arm-linux-ld -Tsimple.lds -o simple_elf $^
	arm-linux-objcopy -O binary -S simple_elf $@
	arm-linux-objdump -D -m arm simple_elf > simple.dis
	
%.o:%.c
	arm-linux-gcc -Wall -fno-builtin -nostdinc -g -c -o $@ $< -Iprintf/include

%.o:%.S
	arm-linux-gcc -Wall -fno-builtin -nostdinc -g -c -o $@ $< 

clean:
	rm -f simple.bin simple_elf simple.dis *.o
	