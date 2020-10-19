#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "dvp.h"
#include "fpioa.h"
#include "lcd.h"
#include "ov2640.h"
#include "plic.h"
#include "sysctl.h"
#include "uarths.h"
#include "st7789.h"
#include "dvp_cam.h"
#include "utils.h"
#include "kpu.h"
#include "l_conv.h"
#include "sleep.h"
#include "encoding.h"
#include "gpiohs.h"
#include "pin_config.h"
#include "dvp_cam.h"


int key_flag = 0;
gpio_pin_value_t key_state = 1;
volatile uint8_t g_ai_done_flag;
uint8_t g_ai_buf_out[320 * 240 * 3] __attribute__((aligned(128)));

/* KPU完成 */
static int kpu_done(void *ctx)
{
	g_ai_done_flag = 1;
	return 0;
}

//  卷积	池化	批归一化	激活	输出偏置
float conv_data[9*3*3] ={
//origin
//R
0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//G
0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//B
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,
};

int demo_index=0;
const float conv_data_demo[4][9*3*3] ={
{	//origin
//R
0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//G
0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//B
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,},
{	//edge
//R
-1,-1,-1,-1,8,-1,-1,-1,-1,
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//G
0,0,0,0,0,0,0,0,0,
-1,-1,-1,-1,8,-1,-1,-1,-1,
0,0,0,0,0,0,0,0,0,
//B
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
-1,-1,-1,-1,8,-1,-1,-1,-1,},
{	//sharp
//R
-1,-1,-1,-1,9,-1,-1,-1,-1,
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//G
0,0,0,0,0,0,0,0,0,
-1,-1,-1,-1,9,-1,-1,-1,-1,
0,0,0,0,0,0,0,0,0,
//B
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
-1,-1,-1,-1,9,-1,-1,-1,-1,},
{	//relievo
//R
2,0,0,0,-1,0,0,0,-1,
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//G
0,0,0,0,0,0,0,0,0,
2,0,0,0,-1,0,0,0,-1,
0,0,0,0,0,0,0,0,0,
//B
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
2,0,0,0,-1,0,0,0,-1,},
};



/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         硬件初始化，绑定GPIO口
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void hardware_init(void)
{
    /* 按键 */
	fpioa_set_function(PIN_KEYPAD_MIDDLE, FUNC_KEYPAD_MIDDLE);

    /* LCD */
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);

    // DVP camera
    fpioa_set_function(PIN_DVP_RST,   FUNC_CMOS_RST);
    fpioa_set_function(PIN_DVP_PWDN,  FUNC_CMOS_PWDN);
    fpioa_set_function(PIN_DVP_XCLK,  FUNC_CMOS_XCLK);
    fpioa_set_function(PIN_DVP_VSYNC, FUNC_CMOS_VSYNC);
    fpioa_set_function(PIN_DVP_HSYNC, FUNC_CMOS_HREF);
    fpioa_set_function(PIN_DVP_PCLK,  FUNC_CMOS_PCLK);
    fpioa_set_function(PIN_DVP_SCL,   FUNC_SCCB_SCLK);
    fpioa_set_function(PIN_DVP_SDA,   FUNC_SCCB_SDA);
    
    // 使能SPI0和DVP
    sysctl_set_spi0_dvp_data(1);
}

/**
* Function       io_set_power
* @author        Gengyue
* @date          2020.05.27
* @brief         设置bank6/bank7电源域1.8V
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
static void io_set_power(void)
{
	/* Set dvp and spi pin to 1.8V */
	sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
	sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

/* 转化图像数据格式，因为摄像头输出到AI的是RGB888格式，而显示屏需要RGB565格式 */
void rgb888_to_565(uint8_t *src_r, uint8_t *src_g, uint8_t *src_b, uint16_t *dst, uint32_t len)
{
	uint32_t i;
	for (i = 0; i < len; i += 2)
	{
		dst[i] = (((uint16_t)(src_r[i + 1] >> 3)) << 11) + 
			(((uint16_t)src_g[i + 1] >> 2) << 5) + 
			(((uint16_t)src_b[i + 1]) >> 3);
		dst[i + 1] = (((uint16_t)(src_r[i] >> 3)) << 11) + 
			(((uint16_t)src_g[i] >> 2) << 5) + 
			(((uint16_t)src_b[i]) >> 3);
	}
}

/* 在原来图像的基础上增加数据（字符） */
void lcd_ram_cpyimg(char *lcd, int lcdw, char *img, int imgw, int imgh, int x, int y)
{
	int i;
	for (i = 0; i < imgh; i++)
	{
		memcpy(lcd + lcdw * 2 * (y + i) + x * 2, img + imgw * 2 * i, imgw * 2);
	}
	return;
}

/* 左上角显示模式 */
void draw_text(void)
{
	char string_buf[8 * 16 * 2 * 16]; //16个字符
	char title[20];

	switch (demo_index)
	{
	case 0:
		sprintf(title, " origin ");
		lcd_ram_draw_string(title, (uint32_t *)string_buf, BLUE, BLACK);
		lcd_ram_cpyimg((char *)g_display_buf, 320, string_buf, strlen(title) * 8, 16, 0, 0);
		break;
	case 1:
		sprintf(title, "  edge  ");
		lcd_ram_draw_string(title, (uint32_t *)string_buf, BLUE, BLACK);
		lcd_ram_cpyimg((char *)g_display_buf, 320, string_buf, strlen(title) * 8, 16, 0, 0);
		break;
	case 2:
		sprintf(title, " sharp  ");
		lcd_ram_draw_string(title, (uint32_t *)string_buf, BLUE, BLACK);
		lcd_ram_cpyimg((char *)g_display_buf, 320, string_buf, strlen(title) * 8, 16, 0, 0);
		break;
	case 3:
		sprintf(title, "relievos");
		lcd_ram_draw_string(title, (uint32_t *)string_buf, BLUE, BLACK);
		lcd_ram_cpyimg((char *)g_display_buf, 320, string_buf, strlen(title) * 8, 16, 0, 0);
		break;
	
	default:
		break;
	}

	// sprintf(title, "% 2d % 2d % 2d", (int)conv_data[3], (int)conv_data[4], (int)conv_data[5]);
	// lcd_ram_draw_string(title, (uint32_t *)string_buf, BLUE, BLACK);
	// lcd_ram_cpyimg((char *)g_display_buf, 320, string_buf, strlen(title) * 8, 16, 0, 16);

}

/* 按键中断回调 */
int key_irq_cb(void *ctx)
{
	key_flag = 1;
	key_state = gpiohs_get_pin(KEYPAD_MIDDLE_GPIONUM);
	return 0;
}

/* 初始化按键 */
void init_key(void)
{
    // 设置按键的GPIO模式为上拉输入
    gpiohs_set_drive_mode(KEYPAD_MIDDLE_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    // 设置按键的GPIO电平触发模式为上升沿和下降沿
    gpiohs_set_pin_edge(KEYPAD_MIDDLE_GPIONUM, GPIO_PE_BOTH);
    // 设置按键GPIO口的中断回调
    gpiohs_irq_register(KEYPAD_MIDDLE_GPIONUM, 1, key_irq_cb, NULL);
}

/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         主函数，程序的入口
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
int main(void)
{
	hardware_init();
	io_set_power();

    /* 设置系统时钟和DVP时钟 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 300000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

	/* 系统中断初始化 */
    plic_init();
	/* 使能系统全局中断 */
    sysctl_enable_irq();
    
	/* 初始化显示屏，并显示一秒图片 */
	printf("LCD init\r\n");
	lcd_init();
	lcd_draw_picture_half(0, 0, 320, 240, gImage_logo);
	sleep(1);

	/* ov2640摄像头初始化 */
	printf("ov2640 init\r\n");
	ov2640_init();

	/* 按键初始化*/
	init_key();

	/* kpu初始化 */
	kpu_task_t task;
	conv_init(&task, CONV_3_3, conv_data);

	printf("KPU TASK INIT, FREE MEM: %ld\r\n", get_free_heap_size());
	printf("Please press the keypad to switch mode\r\n");
	
	while (1)
	{
		while (g_dvp_finish_flag == 0)
			;
		/* 开始运算 */
		conv_run(&task, g_ai_buf_in, g_ai_buf_out, kpu_done);
		
		while (!g_ai_done_flag)
			;
		g_ai_done_flag = 0;
		g_dvp_finish_flag = 0;
		/* 转化成LCD支持的RGB565格式 */
		rgb888_to_565(g_ai_buf_out, g_ai_buf_out + 320 * 240, g_ai_buf_out + 320 * 240 * 2, 
			(uint16_t *)g_display_buf, 320 * 240);
		
		/* 左上角写字母提示是哪个模式 */
		draw_text();
		/* 显示图像 */
		lcd_draw_picture(0, 0, 320, 240, g_display_buf);
		
		if (key_flag) //使用按键选择的卷积核
		{
			if (key_state == 0) //按下
			{
				msleep(20); //延迟去抖
				key_flag = 0;
				demo_index = (demo_index + 1) % 4;
				memcpy((void *)conv_data, (void *)(conv_data_demo[demo_index]), 
					3 * 3 * 3 * 3 * sizeof(float));
				conv_init(&task, CONV_3_3, conv_data);
			}
			else //弹起
			{
				msleep(20); //延迟去抖
				key_flag = 0;
			}
		}
	}
	return 0;
}
