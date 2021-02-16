#include <stdio.h>
#include <string.h>
#include "dmac.h"
#include "unistd.h"
#include "fpioa.h"
#include "sdcard.h"
#include "gpiohs.h"
#include "ff.h"
#include "i2s.h"
#include "uarths.h"
#include "bsp.h"
#include "lcd.h"
#include "st7789.h"
#include "sysctl.h"
#include "uarths.h"
#include "ov2640.h"
#include "dvp_cam.h"
#include "pin_config.h"
#include "plic.h"
#include "sleep.h"
#include "rgb2bmp.h"
#include "iomem.h"
#include "jpeg_encode.h"
#include "picojpeg.h"
#include "picojpeg_util.h"
#include "region_layer.h"
#include "kpu.h"

#include "w25qxx.h"
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"


#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL

// 图像质量，60为最高
#define CONFIG_JPEG_COMPRESS_QUALITY 60
// jpeg区块长度
#define CONFIG_JPEG_BUF_LEN 64
// jpeg图片宽
#define CONFIG_CAMERA_RESOLUTION_WIDTH 320
// jpeg图片长
#define CONFIG_CAMERA_RESOLUTION_HEIGHT 240
// 转换jpeg图片缓冲区
static uint8_t jpeg_buf[CONFIG_JPEG_BUF_LEN * 1024];
static jpeg_encode_t jpeg_src, jpeg_out;
// 统计拍照次数
volatile uint32_t g_count;
// 保存图片标志位
volatile uint8_t g_save_flag;

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

void hardware_init(void)
{
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
    
        /* KEY */
    fpioa_set_function(PIN_KEY, FUNC_KEY);

    /* SD card */
    fpioa_set_function(PIN_TF_MISO, FUNC_TF_SPI_MISO);
    fpioa_set_function(PIN_TF_CLK,  FUNC_TF_SPI_CLK);
    fpioa_set_function(PIN_TF_MOSI, FUNC_TF_SPI_MOSI);
    fpioa_set_function(PIN_TF_CS,   FUNC_TF_SPI_CS);

    // 使能SPI0和DVP
    sysctl_set_spi0_dvp_data(1);

}

int key_irq_cb(void* ctx)
{
    g_save_flag = 1;

    return 0;
}

void init_key(void)
{
    /* 设置按键的GPIO模式为上拉输入 */
    gpiohs_set_drive_mode(KEY_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    /* 设置按键的GPIO电平触发模式为下降沿触发 */
    gpiohs_set_pin_edge(KEY_GPIONUM, GPIO_PE_FALLING);
    /* 设置按键GPIO口的中断回调 */
    gpiohs_irq_register(KEY_GPIONUM, 2, key_irq_cb, &g_count);
}

static int convert_image2jpeg(uint8_t *image, int Quality)
{
    uint64_t v_tim;
    v_tim = sysctl_get_time_us();

    jpeg_src.w = CONFIG_CAMERA_RESOLUTION_WIDTH;
    jpeg_src.h = CONFIG_CAMERA_RESOLUTION_HEIGHT;
    jpeg_src.bpp = 2;
    jpeg_src.data = image;

    jpeg_out.w = jpeg_src.w;
    jpeg_out.h = jpeg_src.h;
    jpeg_out.bpp = CONFIG_JPEG_BUF_LEN * 1024;
    jpeg_out.data = jpeg_buf;

    v_tim = sysctl_get_time_us();
    uint8_t ret = jpeg_compress(&jpeg_src, &jpeg_out, Quality, 0);
    if (ret == 0)
    {
        printf("jpeg encode use %ld us\n", sysctl_get_time_us() - v_tim);
        printf("w:%d\th:%d\tbpp:%d\n", jpeg_out.w, jpeg_out.h, jpeg_out.bpp);
        printf("jpeg encode success!\n");
    }
    else
    {
        printf("jpeg encode failed\n");
    }

    return ret;
}

int save_jpeg_sdcard(uint8_t *image_addr, const char *filename, int img_quality)
{
    FIL file;
    FRESULT ret = FR_OK;
    uint32_t ret_len = 0;

    if (convert_image2jpeg(image_addr, img_quality) == 0)
    {

        if ((ret = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
        {
            printf("create file %s err[%d]", filename, ret);
            return ret;
        }

        f_write(&file, jpeg_out.data, jpeg_out.bpp, &ret_len);

        f_close(&file);
        printf("Save jpeg image %s\n", filename);
        return 1;
    }
    else
    {
        return 0;
    }
}

static int check_sdcard(void)
{
    uint8_t status;

    printf("/******************check_sdcard*****************/\n");
    status = sd_init();
    printf("sd init :%d\n", status);
    if (status != 0)
    {
        return status;
    }

    printf("CardCapacity:%.1fG \n", (double)cardinfo.CardCapacity / 1024 / 1024 / 1024);
    printf("CardBlockSize:%d\n", cardinfo.CardBlockSize);
    return 0;
}

static int check_fat32(void)
{
    static FATFS sdcard_fs;
    FRESULT status;
    DIR dj;
    FILINFO fno;

    printf("/********************check_fat32*******************/\n");
    status = f_mount(&sdcard_fs, _T("0:"), 1);
    printf("mount sdcard:%d\n", status);
    if (status != FR_OK)
        return status;

    printf("printf filename\n");
    status = f_findfirst(&dj, &fno, _T("0:"), _T("*"));
    while (status == FR_OK && fno.fname[0])
    {
        if (fno.fattrib & AM_DIR)
            printf("dir:%s\n", fno.fname);
        else
            printf("file:%s\n", fno.fname);
        status = f_findnext(&dj, &fno);
    }
    f_closedir(&dj);
    return 0;
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

    // 初始化按键key
    init_key();
    // 初始化LCD
    lcd_init();
    /* 设置拍照flag为0 */
    g_save_flag = 0;
    if (check_sdcard())
    {
        printf("SD card err\n");
        return -1;
    }

    if (check_fat32())
    {
        printf("FAT32 err\n");
        return -1;
    }
    lcd_draw_string(10, 10, "Object detection capture demo start !", RED);
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
        uint64_t before_run;
        uint64_t after_run;
        before_run = sysctl_get_time_us();
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

        if (g_save_flag)
        {
            // 根据按键次数生成.jpeg格式文件
            char filename[16];
            sprintf(filename, "0:%08d.jpeg", g_count++);
            // 保存图片到sd卡中
            if(!save_jpeg_sdcard((uint8_t*)display_buf_addr, filename, CONFIG_JPEG_COMPRESS_QUALITY))
            {
                --g_count;
                printf("save img fail !\n");
            }
            g_save_flag = 0;
        }
    }

    return 0;
}
