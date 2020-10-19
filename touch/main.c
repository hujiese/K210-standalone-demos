/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        触摸板读取数据
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 通过串口，把当前触摸的坐标（X,Y的值）打印出来。
*/
#include "sleep.h"
#include "gpiohs.h"
#include "lcd.h"
#include "sysctl.h"
#include "ft6236u.h"
#include "st7789.h"
#include "pin_config.h"

extern const unsigned char gImage_logo[153608];

/**
* Function       io_set_power
* @author        Gengyue
* @date          2020.05.27
* @brief         设置bank6的电源域1.8V
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void io_set_power(void)
{
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
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
    
    /* 使能SPI0和DVP */
    sysctl_set_spi0_dvp_data(1);

    /* I2C FT6236 */
    // fpioa_set_function(PIN_FT_RST, FUNC_FT_RST);
    fpioa_set_function(PIN_FT_INT, FUNC_FT_INT);
    fpioa_set_function(PIN_FT_SCL, FUNC_FT_SCL);
    fpioa_set_function(PIN_FT_SDA, FUNC_FT_SDA);
}

/**
* Function       lcd_clear_coord
* @author        Gengyue
* @date          2020.05.27
* @brief         清除坐标数据
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void lcd_clear_coord(void)
{
    uint32_t color = 0xFFFFFFFF;
    uint8_t x1 = 120;
    uint8_t y1 = 200;
    uint8_t width = 100;
    uint8_t height = 16;
    
    lcd_set_area(x1, y1, x1 + width - 1, y1 + height - 1);
    tft_fill_data(&color, width * height / 2);
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
    /* 硬件引脚初始化 */
    hardware_init();

    /* 设置IO口电压 */
    io_set_power();

    /* 系统中断初始化，并使能全局中断 */
    plic_init();
    sysctl_enable_irq();
    
    /* 初始化LCD */
    lcd_init();

    /* 显示图片 */
    uint16_t * img = &gImage_logo; 
    lcd_draw_picture_half(0, 0, 320, 240, img);
    sleep(1);
    lcd_draw_string(16, 40, "Hello Yahboom", RED);
    lcd_draw_string(16, 60, "Nice to meet you!", BLUE);
    
    /* 初始化触摸板 */
    ft6236_init();
    printf("Hi!Please touch the screen to get coordinates!\n");
    lcd_draw_string(16, 180, "Please touch the screen to get coord!", RED);

    /* 显示的坐标 */
    char coord[30];
    uint8_t is_refresh = 0;

    while (1)
    {
        /* 刷新数据位，清空上次显示的数据 */
        if (is_refresh)
        {
            lcd_clear_coord();
            is_refresh = 0;
        }
        
        /* 如果触摸到触摸屏的时候 */
        if (ft6236.touch_state & TP_COORD_UD)
        {
            ft6236.touch_state &= ~TP_COORD_UD;
            /* 扫描触摸屏 */
            ft6236_scan();
            /* 串口打印X Y 坐标 */
            printf("X=%d, Y=%d \n ", ft6236.touch_x, ft6236.touch_y);
            sprintf(coord, "(%d, %d)", ft6236.touch_x, ft6236.touch_y);

            lcd_draw_string(120, 200, coord, BLUE);
            is_refresh = 1;
        }
        /* 延迟80毫秒保证屏幕数据正常刷新 */
        msleep(80);
    }

    return 0;
}
