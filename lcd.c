#include "include.h"
#include "s3c24xx.h"

#define MODE_TFT_8BIT_480272 	 (0x410c)
#define MODE_TFT_16BIT_480272    (0x4110)

#define BPPMODE_1BPP    0x8
#define BPPMODE_2BPP    0x9
#define BPPMODE_4BPP    0xA
#define BPPMODE_8BPP    0xB
#define BPPMODE_16BPP   0xC
#define BPPMODE_24BPP   0xD

#define LCDTYPE_TFT     0x3

#define ENVID_DISABLE   0
#define ENVID_ENABLE    1

#define FORMAT8BPP_5551 0
#define FORMAT8BPP_565  1

#define HSYNC_NORM      0
#define HSYNC_INV       1

#define VSYNC_NORM      0
#define VSYNC_INV       1

#define VDEN_NORM		0
#define VDEN_INV		1

#define BSWP            1
#define HWSWP           1

#define LCDFRAMEBUFFER 0x30400000

#define LOWER21BITS(n)  ((n) & 0x1fffff)

#define GPB0_tout0  (2<<(0*2))
#define GPB0_out    (1<<(0*2))
#define GPB1_out    (1<<(1*2))

#define GPB0_MSK    (3<<(0*2))
#define GPB1_MSK    (3<<(1*2))

unsigned int fb_base_addr;
unsigned int bpp;
unsigned int xsize;
unsigned int ysize;

static const unsigned short DEMO256pal[]={
    0x0b5e,0xce9a,0xffd9,0x9d99,0xb63a,0xae7c,0xdd71,0x6c57,0xfd4d,0x00ae,0x9c4d,0xb5f8,0xad96,0x0131,0x0176,0xefff,0xcedd,0x9556,0xe4bf,0x00b6,0x22b7,0x002b,0x89de,0x002c,0x57df,0xab5f,0x3031,0x14bf,0x797e,0x5391,0x93ab,0x7239,0x7453,0xafdf,0x71b9,0x8c92,0x014d,0x302e,0x5175,0x0029,0x0969,0x004e,0x2a6d,0x0021,0x3155,0x4b6e,0xd677,0xf6b6,0x9b5f,0x4bb5,0xffd5,0x0027,0xdfdf,0x74d8,0x1256,0x6bcd,0x9b08,0x2ab2,0xbd72,0x84b5,0xfe52,0xd4ad,0x00ad,0xfffc,0x422b,0x73b0,0x0024,0x5246,0x8e5e,0x28b3,0x0050,0x3b52,0x2a4a,0x3a74,0x8559,0x3356,0x1251,0x9abf,0x4034,0x40b1,
    0x8cb9,0x00b3,0x5c55,0xdf3d,0x61b7,0x1f5f,0x00d9,0x4c59,0x0926,0xac3f,0x925f,0x85bc,0x29d2,0xc73f,0xef5c,0xcb9f,0x827b,0x5279,0x4af5,0x01b9,0x4290,0xf718,0x126d,0x21a6,0x515e,0xefbd,0xd75e,0x42ab,0x00aa,0x10b3,0x7349,0x63b5,0x61a3,0xaadf,0xcb27,0x87df,0x6359,0xc7df,0x4876,0xb5bc,0x4114,0xfe2e,0xef5e,0x65be,0x43b9,0xe5df,0x21c9,0x7d16,0x6abb,0x5c11,0x49f7,0xbc0b,0x9e1a,0x3b0f,0x202b,0xff12,0x821b,0x842f,0xbccf,0xdefb,0x8a3e,0x68fa,0xa4f1,0x38ae,0x28b7,0x21ad,0x31d7,0x0073,0x182b,0x1831,0x3415,0xbdf6,0x2dbf,0x0a5d,0xc73d,0x182c,0x293e,0x7b3d,0x643d,0x3cbd,
    0x92dd,0x09d4,0x1029,0x7cdd,0x6239,0x182e,0x5aea,0x11eb,0x8abc,0x7bfa,0x00a7,0x2153,0x1853,0x1318,0x0109,0x54fa,0x72a7,0x89e3,0x01cf,0x3a07,0x7b17,0x1a14,0x2150,0x23dc,0x4142,0x1b33,0x00a4,0xf6df,0x08fc,0x18ae,0x3a7e,0x18d1,0xa51c,0xff5a,0x1a0f,0x28fa,0xdfbe,0x82de,0x60d7,0x1027,0x48fa,0x5150,0x6213,0x89d6,0x110d,0x9bbb,0xbedd,0x28e1,0x1925,0xf449,0xaa79,0xd5f4,0x693c,0x110a,0x2889,0x08a2,0x923d,0x10a6,0xd9bc,0x5b2e,0x32ec,0xcf7f,0x1025,0x2148,0x74b4,0x6d59,0x9d14,0x0132,0x00f0,0x56bf,0x00f1,0xffff,0x0173,0x0133,0x00b0,0x00b1,0xf7ff,0x08b1,0xfffe,0x08b0,
    0x0171,0xf7bf,0x10f3,0xf7fe,0x08ef,0x1192,0xefbe,0x1131,0x2177,0xff9f,0x1116,0xffbc,0x5914,0x22ef,0xb285,0xa6df,
};

void Lcd_Port_Init(void)
{
    GPCUP   = 0xffffffff;   // 禁止内部上拉
    GPCCON  = 0xaaaaaaaa;   // GPIO管脚用于VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND 

	GPDUP   = 0xffffffff;   // 禁止内部上拉
	GPDCON  = 0xaaaaaaaa;   // GPIO管脚用于VD[23:8]

	GPBCON &= ~(GPB0_MSK);  // backlight
    GPBCON |= GPB0_out;
    GPBDAT &= ~(1<<0);		// backlight off
}

void Tft_Lcd_Init(int type)
{	
    switch(type)
    {
		case MODE_TFT_8BIT_480272:
        /* 
         * 设置LCD控制器的控制寄存器LCDCON1~5
         * 1. LCDCON1:
         *    设置VCLK的频率：VCLK(Hz) = HCLK/[(CLKVAL+1)x2] = 10MHZ
         *    选择LCD类型: TFT LCD   
         *    设置显示模式: 8BPP
         *    先禁止LCD信号输出
         * 2. LCDCON2/3/4:
         *    设置控制信号的时间参数
         *    设置分辨率，即行数及列数
         * 现在，可以根据公式计算出显示器的频率：
         * 当HCLK=100MHz时，
         * Frame Rate = 1/[{(VSPW+1)+(VBPD+1)+(LIINEVAL+1)+(VFPD+1)}x
         *              {(HSPW+1)+(HBPD+1)+(HFPD+1)+(HOZVAL+1)}x
         *              {2x(CLKVAL+1)/(HCLK)}]
         *            = 60Hz
         * 3. LCDCON5:
         *    设置显示模式为8BPP时，调色板中的数据格式: 5:6:5
         *    设置HSYNC、VSYNC脉冲的极性(这需要参考具体LCD的接口信号): 反转
         *    字节交换使能
         */
        LCDCON1 = (4<<8) | (LCDTYPE_TFT<<5) | (BPPMODE_8BPP<<1) | (ENVID_DISABLE<<0);

		//VBPD = 1,LINEVAL = 271 VFPD = 1 VSPW = 9 
        LCDCON2 = (1<<24) | (271<<14) | (1<<6) | (9);
		//HPBD = 1 HOZVAL = 479 HFPD = 1
        LCDCON3 = (1<<19) | (479<<8) | (1);
		//HSPW = 40
        LCDCON4 = 40;
		//[{(VSPW+1)+(VBPD+1)+(LIINEVAL+1)+(VFPD+1)}x{(HSPW+1)+(HBPD+1)+(HFPD+1)+(HOZVAL+1)} = 286x525 = 150150
		//10M / 150150 = 66
		
        LCDCON5 = (FORMAT8BPP_565<<11) | (HSYNC_INV<<9) | (VSYNC_INV<<8) | (BSWP<<1);
 
        /*
         * 设置LCD控制器的地址寄存器LCDSADDR1~3
         * 帧内存与视口(view point)完全吻合，
         * 图像数据格式如下(8BPP时，帧缓冲区中的数据为调色板中的索引值)：
         *         |----PAGEWIDTH----|
         *    y/x  0   1   2       239
         *     0   idx idx idx ... idx
         *     1   idx idx idx ... idx
         * 1. LCDSADDR1:
         *    设置LCDBANK、LCDBASEU
         * 2. LCDSADDR2:
         *    设置LCDBASEL: 帧缓冲区的结束地址A[21:1]
         * 3. LCDSADDR3:
         *    OFFSIZE等于0，PAGEWIDTH等于(240/2)
         */
        LCDSADDR1 = ((LCDFRAMEBUFFER>>22)<<21) | LOWER21BITS(LCDFRAMEBUFFER>>1);
        LCDSADDR2 = LOWER21BITS((LCDFRAMEBUFFER+(480)*(272)*1)>>1);
        LCDSADDR3 = (0<<11) | (480/2);

        /* 禁止临时调色板寄存器 */
        TPAL = 0;

        fb_base_addr = LCDFRAMEBUFFER;
        bpp = 8;
        xsize = 480;
        ysize = 272;
        
        break;

    case MODE_TFT_16BIT_480272:
        /* 
         * 设置LCD控制器的控制寄存器LCDCON1~5
         * 1. LCDCON1:
         *    设置VCLK的频率：VCLK(Hz) = HCLK/[(CLKVAL+1)x2]
         *    选择LCD类型: TFT LCD   
         *    设置显示模式: 16BPP
         *    先禁止LCD信号输出
         * 2. LCDCON2/3/4:
         *    设置控制信号的时间参数
         *    设置分辨率，即行数及列数
         * 现在，可以根据公式计算出显示器的频率：
         * 当HCLK=100MHz时，
         * Frame Rate = 1/[{(VSPW+1)+(VBPD+1)+(LIINEVAL+1)+(VFPD+1)}x
         *              {(HSPW+1)+(HBPD+1)+(HFPD+1)+(HOZVAL+1)}x
         *              {2x(CLKVAL+1)/(HCLK)}]
         *            = 60Hz
         * 3. LCDCON5:
         *    设置显示模式为16BPP时的数据格式: 5:6:5
         *    设置HSYNC、VSYNC脉冲的极性(这需要参考具体LCD的接口信号): 反转
         *    半字(2字节)交换使能
         */
        LCDCON1 = (4<<8) | (LCDTYPE_TFT<<5) | (BPPMODE_16BPP<<1) | (ENVID_DISABLE<<0);
        LCDCON2 = (1<<24) | (271<<14) | (1<<6) | (9);
        LCDCON3 = (1<<19) | (479<<8) | (1);
        LCDCON4 = 40;
        LCDCON5 = (FORMAT8BPP_565<<11) | (HSYNC_INV<<9) | (VSYNC_INV<<8) | (HWSWP<<1);

        /*
         * 设置LCD控制器的地址寄存器LCDSADDR1~3
         * 帧内存与视口(view point)完全吻合，
         * 图像数据格式如下：
         *         |----PAGEWIDTH----|
         *    y/x  0   1   2       239
         *     0   rgb rgb rgb ... rgb
         *     1   rgb rgb rgb ... rgb
         * 1. LCDSADDR1:
         *    设置LCDBANK、LCDBASEU
         * 2. LCDSADDR2:
         *    设置LCDBASEL: 帧缓冲区的结束地址A[21:1]
         * 3. LCDSADDR3:
         *    OFFSIZE等于0，PAGEWIDTH等于(480*2/2)
         */
        LCDSADDR1 = ((LCDFRAMEBUFFER>>22)<<21) | LOWER21BITS(LCDFRAMEBUFFER>>1);
        LCDSADDR2 = LOWER21BITS((LCDFRAMEBUFFER+(480)*(272)*2)>>1);
        LCDSADDR3 = (0<<11) | (480*2/2);

        /* 禁止临时调色板寄存器 */
        TPAL = 0;

        fb_base_addr = LCDFRAMEBUFFER;
        bpp = 16;
        xsize = 480;
        ysize = 272;

        break;
	}
	
	
}

void Lcd_EnvidOnOff(int onoff)
{
    if (onoff == 1)
    {
        LCDCON1 |= 1;         // ENVID ON
		//GPBDAT |= (1<<0);			// Power on
    }
    else
    {
        LCDCON1 &= 0x3fffe;  // ENVID Off
	    //GPBDAT &= ~(1<<0);	 // Power off
    }
} 

void Lcd_Palette8Bit_Init(void)
{
    int i;  
    volatile unsigned int *palette;

	LCDCON1 &= ~0x01;	// stop lcd controller
    
    LCDCON5 |= (FORMAT8BPP_565<<11); // 设置调色板中数据格式为5:6:5

    palette = (volatile unsigned int *)PALETTE;
    for (i = 0; i < 256; i++)
        *palette++ = DEMO256pal[i];

	LCDCON1 |= 0x01;	// re-enable lcd controller
}

/*
 * 使用临时调色板寄存器输出单色图像
 * 输入参数：
 *     color: 颜色值，格式为0xRRGGBB
 */
void ClearScrWithTmpPlt(UINT32 color)
{
    TPAL = (1<<24)|((color & 0xffffff)<<0);
}

/*
 * 停止使用临时调色板寄存器
 */
void DisableTmpPlt(void)
{
    TPAL = 0;
}


void PutPixel(UINT32 x, UINT32 y, UINT32 color)
{
    UINT8 red,green,blue;

    switch (bpp){
        case 16:
        {
            UINT16 *addr = (UINT16 *)fb_base_addr + (y * xsize + x);
            red   = (color >> 19) & 0x1f;
            green = (color >> 10) & 0x3f;
            blue  = (color >>  3) & 0x1f;
            color = (red << 11) | (green << 5) | blue; // 格式5:6:5
            *addr = (UINT16) color;
            break;
        }
        
        case 8:
        {
            UINT8 *addr = (UINT8 *)fb_base_addr + (y * xsize + x);
            *addr = (UINT8) color;
            break;
        }

        default:
            break;
    }
}

void DrawLine(int x1,int y1,int x2,int y2,int color)
{
    int dx,dy,e;
    dx=x2-x1; 
    dy=y2-y1;
    
    if(dx>=0)
    {
        if(dy >= 0) // dy>=0
        {
            if(dx>=dy) // 1/8 octant
            {
                e=dy-dx/2;
                while(x1<=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1+=1;e-=dx;}   
                    x1+=1;
                    e+=dy;
                }
            }
            else        // 2/8 octant
            {
                e=dx-dy/2;
                while(y1<=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1+=1;e-=dy;}   
                    y1+=1;
                    e+=dx;
                }
            }
        }
        else           // dy<0
        {
            dy=-dy;   // dy=abs(dy)

            if(dx>=dy) // 8/8 octant
            {
                e=dy-dx/2;
                while(x1<=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1-=1;e-=dx;}   
                    x1+=1;
                    e+=dy;
                }
            }
            else        // 7/8 octant
            {
                e=dx-dy/2;
                while(y1>=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1+=1;e-=dy;}   
                    y1-=1;
                    e+=dx;
                }
            }
        }   
    }
    else //dx<0
    {
        dx=-dx;     //dx=abs(dx)
        if(dy >= 0) // dy>=0
        {
            if(dx>=dy) // 4/8 octant
            {
                e=dy-dx/2;
                while(x1>=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1+=1;e-=dx;}   
                    x1-=1;
                    e+=dy;
                }
            }
            else        // 3/8 octant
            {
                e=dx-dy/2;
                while(y1<=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1-=1;e-=dy;}   
                    y1+=1;
                    e+=dx;
                }
            }
        }
        else           // dy<0
        {
            dy=-dy;   // dy=abs(dy)

            if(dx>=dy) // 5/8 octant
            {
                e=dy-dx/2;
                while(x1>=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1-=1;e-=dx;}   
                    x1-=1;
                    e+=dy;
                }
            }
            else        // 6/8 octant
            {
                e=dx-dy/2;
                while(y1>=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1-=1;e-=dy;}   
                    y1-=1;
                    e+=dx;
                }
            }
        }   
    }
}

/* 
 * 将屏幕清成单色
 * 输入参数：
 *     color: 颜色值
 *         对于16BPP: color的格式为0xAARRGGBB (AA = 透明度),
 *     需要转换为5:6:5格式
 *         对于8BPP: color为调色板中的索引值，
 *     其颜色取决于调色板中的数值
 */

void ClearScr(UINT32 color)
{   
    UINT32 x,y;
    
    for (y = 0; y < ysize; y++)
        for (x = 0; x < xsize; x++)
            PutPixel(x, y, color);
}


void Test_Lcd_Tft_8bit_480272(void)
{
	Lcd_Port_Init();
	Tft_Lcd_Init(MODE_TFT_8BIT_480272);
	Lcd_Palette8Bit_Init();
	ClearScr(0x0);
	Lcd_EnvidOnOff(1);

	#if 0
	ClearScrWithTmpPlt(0x0000ff);
	printf("press any key to next\r\n");
	getc();
	
	ClearScrWithTmpPlt(0xffffff);
	printf("press any key to next\r\n");
	getc();
    DisableTmpPlt();
	#endif

	printf("press any key to next\r\n");
    getc();
	//ClearScr(0x2);
	int i;
	for(i = 0 ; i < 256 ; i++)
	{
		DrawLine(0	, i  , 480,  i ,i);
	}	
	
	printf("press any key to next\r\n");
    getc();
	DrawLine(0	, 0  , 100, 100  ,128);

	printf("press any key to next\r\n");
	getc();
	DrawLine(480  , 0  , 200  , 200, 110); 

	printf("press any key to next\r\n");
	getc();
	ClearScr(128); 
	
}

void Test_Lcd_Tft_16Bit_480272(void)
{
	Lcd_Port_Init();
	Tft_Lcd_Init(MODE_TFT_16BIT_480272);
	ClearScr(0xffffff);
	pwm0_init();  //lcd backscreen light
	Lcd_EnvidOnOff(1);

	DrawLine(0  , 0  , 479, 0  , 0xff0000);    // 红色
    DrawLine(0  , 0  , 0  , 271, 0x00ff00);    // 绿色
    DrawLine(479, 0  , 479, 271, 0x0000ff);    // 蓝色
    DrawLine(0  , 271, 479, 271, 0xffffff);    // 白色
    DrawLine(0  , 0  , 479, 271, 0xffff00);    // 黄色
    DrawLine(479, 0  , 0  , 271, 0x8000ff);    // 紫色
    DrawLine(240, 0  , 240, 271, 0xe6e8fa);    // 银色
    DrawLine(0  , 136, 479, 136, 0xcd7f32);    // 金色
	
}
