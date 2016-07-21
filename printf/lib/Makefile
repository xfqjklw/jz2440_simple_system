INCLUDEDIR 	:= ../include
#-fno-builtin可避免编译器将printfputs替换成puts
CFLAGS 		:= -Wall -O2 -fno-builtin -g 
CPPFLAGS   	:= -nostdinc -I$(INCLUDEDIR)
objs := div64.o lib1funcs.o ctype.o muldi3.o printf.o string.o vsprintf.o

libc.a: $(objs)
	arm-linux-ar -r -o $@ $^
	
%.o:%.c
	arm-linux-gcc $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o:%.S
	arm-linux-gcc $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f libc.a *.o		
	
