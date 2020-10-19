/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        lcd显示图片
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 把320*240的图片转化成数据格式，然后在LCD上显示。
*/

#include "gpiohs.h"
#include "lcd.h"
#include "st7789.h"
#include "image.h"
#include "sysctl.h"
#include "ov2640.h"
#include "dvp_cam.h"
#include "pin_config.h"
#include "uarths.h"
#include "region_layer.h"
#include "kpu.h"
#include "stdio.h"
#include "bsp.h"


#include "w25qxx.h"
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"


#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL


volatile uint32_t g_ai_done_flag;
//volatile uint8_t g_dvp_finish_flag;
//static image_t kpu_image, kpu_od_image, display_image;

kpu_model_context_t obj_detect_task;
static region_layer_t obj_detect_rl;
static obj_info_t obj_detect_info;
#define ANCHOR_NUM 5
static float anchor[ANCHOR_NUM * 2] = {1.3221, 1.73145, 3.19275, 4.00944, 5.05587, 8.09892, 9.47112, 4.84053, 11.2364, 10.0071};

#define  LOAD_KMODEL_FROM_FLASH  0
#define CLASS_NUMBER 20

#if LOAD_KMODEL_FROM_FLASH
#define KMODEL_SIZE (1501 * 1024)
uint8_t *model_data;
#else
INCBIN(model, "detect.kmodel");
#endif


static int ai_done(void *ctx)
{
    g_ai_done_flag = 1;
    return 0;
}

#if (CLASS_NUMBER > 1)
typedef struct
{
    char *str;
    uint16_t color;
    uint16_t height;
    uint16_t width;
    uint32_t *ptr;
} class_lable_t;

class_lable_t class_lable[CLASS_NUMBER] =
{
    { "aeroplane", NAVY },
    { "bicycle", DARKGREEN },
    { "bird", DARKCYAN },
    { "boat", MAROON },
    { "bottle", PURPLE },
    { "bus", LIGHTGREY },
    { "car", DARKGREY },
    { "cat", BLUE },
    { "chair", RED },
    { "cow", GREEN },
    { "diningtable", WHITE },
    { "dog", RED },
    { "horse", MAGENTA },
    { "motorbike", YELLOW },
    { "person", CYAN },
    { "pottedplant", ORANGE },
    { "sheep", GREENYELLOW },
    { "sofa", PINK },
    { "train", USER_COLOR },
    { "tvmonitor", NAVY }
};

static uint32_t lable_string_draw_ram[115 * 16 * 8 / 2];
#endif

void io_set_power(void)
{
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

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
    // fpioa映射
    /**
    *lcd_cs	    36
    *lcd_rst	37
    *lcd_rs	    38
    *lcd_wr 	39
    **/
    fpioa_set_function(PIN_LCD_CS, FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS, FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR, FUNC_LCD_WR);

    // DVP camera
    fpioa_set_function(PIN_DVP_RST, FUNC_CMOS_RST);
    fpioa_set_function(PIN_DVP_PWDN, FUNC_CMOS_PWDN);
    fpioa_set_function(PIN_DVP_XCLK, FUNC_CMOS_XCLK);
    fpioa_set_function(PIN_DVP_VSYNC, FUNC_CMOS_VSYNC);
    fpioa_set_function(PIN_DVP_HSYNC, FUNC_CMOS_HREF);
    fpioa_set_function(PIN_DVP_PCLK, FUNC_CMOS_PCLK);
    fpioa_set_function(PIN_DVP_SCL, FUNC_SCCB_SCLK);
    fpioa_set_function(PIN_DVP_SDA, FUNC_SCCB_SDA);
    
    // 使能SPI0和DVP
    sysctl_set_spi0_dvp_data(1);

}

static void lable_init(void)
{

#if (CLASS_NUMBER > 1)
	uint8_t index;

	class_lable[0].height = 16;
	class_lable[0].width = 8 * strlen(class_lable[0].str);
	class_lable[0].ptr = lable_string_draw_ram;
	lcd_ram_draw_string(class_lable[0].str, class_lable[0].ptr, BLACK, class_lable[0].color);
	for (index = 1; index < CLASS_NUMBER; index++) {
		class_lable[index].height = 16;
		class_lable[index].width = 8 * strlen(class_lable[index].str);
		class_lable[index].ptr = class_lable[index - 1].ptr + class_lable[index - 1].height * class_lable[index - 1].width / 2;
		lcd_ram_draw_string(class_lable[index].str, class_lable[index].ptr, BLACK, class_lable[index].color);
	}
#endif

}

static void drawboxes(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t class, float prob)
{
    if (x1 >= 320)
        x1 = 319;
    if (x2 >= 320)
        x2 = 319;
    if (y1 >= 240)
        y1 = 239;
    if (y2 >= 240)
        y2 = 239;

    lcd_draw_rectangle(x1, y1, x2, y2, 2, class_lable[class].color);
    lcd_draw_picture(x1 + 1, y1 + 1, class_lable[class].width, class_lable[class].height, class_lable[class].ptr);
}

static void draw_edge(uint32_t *gram, obj_info_t *obj_info, uint32_t index, uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;
    uint32_t *addr1, *addr2, *addr3, *addr4, x1, y1, x2, y2;

    x1 = obj_info->obj[index].x1;
    y1 = obj_info->obj[index].y1;
    x2 = obj_info->obj[index].x2;
    y2 = obj_info->obj[index].y2;

    if (x1 <= 0)
        x1 = 1;
    if (x2 >= 319)
        x2 = 318;
    if (y1 <= 0)
        y1 = 1;
    if (y2 >= 239)
        y2 = 238;

    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 8) / 2;
    addr3 = gram + (320 * (y2 - 1) + x1) / 2;
    addr4 = gram + (320 * (y2 - 1) + x2 - 8) / 2;
    for (uint32_t i = 0; i < 4; i++)
    {
        *addr1 = data;
        *(addr1 + 160) = data;
        *addr2 = data;
        *(addr2 + 160) = data;
        *addr3 = data;
        *(addr3 + 160) = data;
        *addr4 = data;
        *(addr4 + 160) = data;
        addr1++;
        addr2++;
        addr3++;
        addr4++;
    }
    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 2) / 2;
    addr3 = gram + (320 * (y2 - 8) + x1) / 2;
    addr4 = gram + (320 * (y2 - 8) + x2 - 2) / 2;
    for (uint32_t i = 0; i < 8; i++)
    {
        *addr1 = data;
        *addr2 = data;
        *addr3 = data;
        *addr4 = data;
        addr1 += 160;
        addr2 += 160;
        addr3 += 160;
        addr4 += 160;
    }
}

/*系统时钟初始化*/
void sysclock_init(void)
{
    /* 设置系统时钟和DVP时钟 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000UL);
    //sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
}
/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         主函数，程序的入口
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
int main(void)
{
    sysclock_init();   /* 系统时钟初始化*/
    uarths_init();     /* 串口初始化*/
    hardware_init();   /* 硬件引脚初始化*/
    io_set_power();    /* 设置IO口电压*/
    plic_init();       /* 系统中断初始化 */
    lable_init();      /* 层初始化*/
    
     /* flash init */
    printf("flash init\n");
    w25qxx_init(3, 0);
    w25qxx_enable_quad_mode();
#if LOAD_KMODEL_FROM_FLASH
    model_data = (uint8_t*)malloc(KMODEL_SIZE + 255);
    uint8_t *model_data_align = (uint8_t*)(((uintptr_t)model_data+255)&(~255));
    w25qxx_read_data(0xA00000, model_data_align, KMODEL_SIZE, W25QXX_QUAD_FAST);
#else
    uint8_t *model_data_align = model_data;
#endif

    // 初始化LCD
    lcd_init();
    lcd_draw_picture_half(0, 0, 320, 240, logo);
    lcd_draw_string(100, 40, "Hello Yahboom!", RED);
    lcd_draw_string(100, 60, "object detection demo!", BLUE);
    sleep(1);
    /* 初始化摄像头*/
    ov2640_init();



     /* 初始化物体检测模型 */
    if (kpu_load_kmodel(&obj_detect_task, model_data_align) != 0)
    {
        printf("\nmodel init error\n");
        while (1);
    }
    obj_detect_rl.anchor_number = ANCHOR_NUM;
    obj_detect_rl.anchor = anchor;
    obj_detect_rl.threshold = 0.5;
    obj_detect_rl.nms_value = 0.2;
    region_layer_init(&obj_detect_rl, 10, 8, 125, 320, 240);
    /* enable global interrupt */
    sysctl_enable_irq();
    /* system start */
    printf("System start\n");


    while (1)
    {
        g_dvp_finish_flag = 0;
        
        while (!g_dvp_finish_flag)
            ;
            
        /* run obj detect */
		memset(g_ai_od_buf, 127, 320*256*3);
		for (uint32_t cc = 0; cc < 3; cc++)
		{
			memcpy(g_ai_od_buf + 320 * (cc * 256 + (256 - 240) / 2), g_ai_buf_in + cc * 320 * 240, 320 * 240);
		}

        /*运行模型*/
        g_ai_done_flag = 0;
        kpu_run_kmodel(&obj_detect_task, g_ai_od_buf, DMAC_CHANNEL5, ai_done, NULL);
        while(!g_ai_done_flag);
        
        /*获取KPU处理结果*/
        float *output;
        size_t output_size;
        kpu_get_output(&obj_detect_task, 0, (uint8_t **)&output, &output_size);
        
        /*获取输出层的结果*/
        obj_detect_rl.input = output;
        region_layer_run(&obj_detect_rl, &obj_detect_info);
		
        /* 显示视频图像*/
		lcd_draw_picture(0, 0, 320, 240, (uint32_t *)display_buf_addr);

        /* 画识别结果 */
		region_layer_draw_boxes(&obj_detect_rl, drawboxes); 

        
		//g_dvp_finish_flag = 0;

		
    }

    return 0;
}
