#include "include.h"
#include "s3c24xx.h"

#define BUSY            1

#define NAND_SECTOR_SIZE_LP    2048
#define NAND_BLOCK_MASK_LP     (NAND_SECTOR_SIZE_LP - 1)




/* 等待NAND Flash就绪 */
static void s3c2440_wait_idle(void)
{
    int i;
    while(!(NFSTAT & BUSY))
        for(i=0; i<10; i++);
}

/* 发出片选信号 */
static void s3c2440_nand_select_chip(void)
{
    int i;
    NFCONT &= ~(1<<1);
    for(i=0; i<10; i++);    
}

/* 取消片选信号 */
static void s3c2440_nand_deselect_chip(void)
{
    NFCONT |= (1<<1);
}

/* 发出命令 */
static void s3c2440_write_cmd(int cmd)
{
    NFCMD = cmd;
}


static void s3c2440_write_addr_lp(unsigned int addr)
{
	int i;
	int col, page;

	col = addr & NAND_BLOCK_MASK_LP;
	page = addr / NAND_SECTOR_SIZE_LP;
	
	NFADDR = col & 0xff;			/* Column Address A0~A7 */
	for(i=0; i<10; i++);		
	NFADDR = (col >> 8) & 0x0f; 	/* Column Address A8~A11 */
	for(i=0; i<10; i++);
	NFADDR = page & 0xff;			/* Row Address A12~A19 */
	for(i=0; i<10; i++);
	NFADDR = (page >> 8) & 0xff;	/* Row Address A20~A27 */
	for(i=0; i<10; i++);
	NFADDR = (page >> 16) & 0x03;	/* Row Address A28~A29 */
	for(i=0; i<10; i++);
}


/* 读取数据 */
static unsigned char s3c2440_read_data(void)
{
    return NFDATA;
}

/* 复位 */
static void s3c2440_nand_reset(void)
{
    s3c2440_nand_select_chip();
    s3c2440_write_cmd(0xff);  // 复位命令
    s3c2440_wait_idle();
    s3c2440_nand_deselect_chip();
}


/* 初始化NAND Flash */
void nand_init(void)
{
#define TACLS   0
#define TWRPH0  3
#define TWRPH1  0


	/* 设置时序 */
    NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
    /* 使能NAND Flash控制器, 初始化ECC, 禁止片选 */
    NFCONT = (1<<4)|(1<<1)|(1<<0);
    
    /* 复位NAND Flash */
    s3c2440_nand_reset();
}



void nand_read(unsigned char *buf, unsigned long start_addr,int size)
{
    int i, j;
	
	if ((start_addr & NAND_BLOCK_MASK_LP) || (size & NAND_BLOCK_MASK_LP)) {
        return ;    /* 地址或长度不对齐 */
    }
	s3c2440_nand_select_chip();	
    for(i=0; i<10; i++);

    for(i=start_addr; i < (start_addr + size);) 
	{
		s3c2440_write_cmd(0);
		s3c2440_write_addr_lp(i);
		s3c2440_write_cmd(0x30);	
		s3c2440_wait_idle();
		
		for(j=0; j < NAND_SECTOR_SIZE_LP; j++, i++) 
		{
			*buf = s3c2440_read_data();
			buf++;
      	}
    }
	
	s3c2440_nand_deselect_chip();
    return ;
}


