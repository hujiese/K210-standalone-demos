/**
* @par  Copyright (C): 2016-2022, SCUT
* @file         main.c
* @author       jackster
* @version      V1.0
* @date         2020.10.19
* @brief        摄像头显示当前画面，拍照存入SD卡中
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 摄像头采集当前画面，然后在LCD上显示;通过boot按键拍照将图片保存为jpeg格式存入sd卡根目录中。
*/
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

/**
* Function       io_set_power
* @author        jackster
* @date          2020.10.19
* @brief         设置摄像头和显示器电源域电压值
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void io_set_power(void)
{
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

/**
* Function       hardware_init
* @author        jackster
* @date          2020.10.19
* @brief         硬件初始化，绑定GPIO口
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void hardware_init(void)
{
    /* lcd */
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);

    /* DVP camera */
    fpioa_set_function(PIN_DVP_RST,   FUNC_CMOS_RST);
    fpioa_set_function(PIN_DVP_PWDN,  FUNC_CMOS_PWDN);
    fpioa_set_function(PIN_DVP_XCLK,  FUNC_CMOS_XCLK);
    fpioa_set_function(PIN_DVP_VSYNC, FUNC_CMOS_VSYNC);
    fpioa_set_function(PIN_DVP_HSYNC, FUNC_CMOS_HREF);
    fpioa_set_function(PIN_DVP_PCLK,  FUNC_CMOS_PCLK);
    fpioa_set_function(PIN_DVP_SCL,   FUNC_SCCB_SCLK);
    fpioa_set_function(PIN_DVP_SDA,   FUNC_SCCB_SDA);

    /* KEY */
    fpioa_set_function(PIN_KEY, FUNC_KEY);

    /* SD card */
    fpioa_set_function(PIN_TF_MISO, FUNC_TF_SPI_MISO);
    fpioa_set_function(PIN_TF_CLK,  FUNC_TF_SPI_CLK);
    fpioa_set_function(PIN_TF_MOSI, FUNC_TF_SPI_MOSI);
    fpioa_set_function(PIN_TF_CS,   FUNC_TF_SPI_CS);

    /* 使能SPI0和DVP */
    sysctl_set_spi0_dvp_data(1);
}

/**
* Function       key_irq_cb
* @author        jackster
* @date          2020.10.19
* @brief         按键key中断回调函数
* @param[in]     ctx 回调参数
* @param[out]    void
* @retval        0
* @par History   无
*/
int key_irq_cb(void* ctx)
{
    g_save_flag = 1;

    return 0;
}

/**
* Function       init_key
* @author        jackster
* @date          2020.10.19
* @brief         初始化按键key
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void init_key(void)
{
    /* 设置按键的GPIO模式为上拉输入 */
    gpiohs_set_drive_mode(KEY_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    /* 设置按键的GPIO电平触发模式为下降沿触发 */
    gpiohs_set_pin_edge(KEY_GPIONUM, GPIO_PE_FALLING);
    /* 设置按键GPIO口的中断回调 */
    gpiohs_irq_register(KEY_GPIONUM, 2, key_irq_cb, &g_count);
}

/**
* Function       convert_image2jpeg
* @author        jackster
* @date          2020.10.19
* @brief         将 rgb565图像字节数组转为jpeg图片数组
* @param[in]     image rgb565图像字节数组;Quality 转换质量
* @param[out]    ret 值
* @retval        void
* @par History   无
*/
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

/**
* Function       save_jpeg_sdcard
* @author        jackster
* @date          2020.10.19
* @brief         将 rgb565图像字节数组转换为jpeg格式图片并写入sd卡中
* @param[in]     image_addr 摄像头拍摄rgb565图片地址;filename 文件全路径名;img_quality 图像质量
* @param[out]    成功返回1, 失败返回0
* @retval        void
* @par History   无
*/
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

/**
* Function       check_sdcard
* @author        jackster
* @date          2020.10.19
* @brief         检测TF是否正常
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
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

/**
* Function       check_fat32
* @author        jackster
* @date          2020.10.19
* @brief         检测TF的格式是否FAT32
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
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

/**
* Function       main
* @author        jackster
* @date          2020.10.19
* @brief         主函数，程序的入口
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
int main(void)
{
    /* 硬件引脚初始化 */
    hardware_init();
    
    /* 设置IO口电压 */
    io_set_power();

    /* 设置系统时钟和DVP时钟 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 300000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

    /* 系统中断初始化，使能全局中断*/
    plic_init();
    sysctl_enable_irq();
    
    /* 初始化LCD */
    lcd_init();

    // 初始化按键key
    init_key();

    /* 初始化摄像头 */
    ov2640_init();

    /* 设置拍照flag为0 */
    g_save_flag = 0;
    
    g_ram_mux = 0;

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

    while (1)
    {
        /* 等待摄像头采集结束，然后清除结束标志 */
        while (g_dvp_finish_flag == 0)
            ;
        g_ram_mux ^= 0x01;
        g_dvp_finish_flag = 0;

        if (g_save_flag)
        {
            // 根据按键次数生成.jpeg格式文件
            char filename[16];
            sprintf(filename, "0:%08d.jpeg", g_count++);
            // 保存图片到sd卡中
            if(!save_jpeg_sdcard((uint8_t*)(g_ram_mux ? display_buf_addr1 : display_buf_addr2), filename, CONFIG_JPEG_COMPRESS_QUALITY))
            {
                --g_count;
                printf("save img fail !\n");
            }
            g_save_flag = 0;
        }

        /* 显示画面 */
        lcd_draw_picture(0, 0, 320, 240, g_ram_mux ? display_buf_addr1 : display_buf_addr2);
    }
    iomem_free(display_buf_addr1);
    iomem_free(display_buf_addr2);

    return 0;
}


