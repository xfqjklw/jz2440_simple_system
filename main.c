#include "include.h"

int main()
{    

	uart0_init();
	led_init();
	key_init();

	printf("---------------------------------------\r\n");
	printf("           system start %s             \r\n",VERSION);
	printf("---------------------------------------\r\n");

 	while(1)
	{
 	}
	return 0;
}
