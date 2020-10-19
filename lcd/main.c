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
* version:	V1.0: LCD显示图片和字符。
*/
#include "sleep.h"
#include "gpiohs.h"
#include "lcd.h"
#include "st7789.h"
#include "sysctl.h"
#include "pin_config.h"


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
    *PIN_LCD_CS	    36
    *PIN_LCD_RST	37
    *PIN_LCD_RS	    38
    *PIN_LCD_WR 	39
    **/
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);
    
    /* 使能SPI0和DVP数据 */
    sysctl_set_spi0_dvp_data(1);

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
    
    /* 初始化LCD */
    lcd_init();

    /* 显示图片 */
    lcd_draw_picture_half(0, 0, 320, 240, gImage_logo);
    sleep(1);

    /* 显示字符 */
    lcd_draw_string(16, 40, "Hello World!", RED);
    lcd_draw_string(16, 60, "Nice to meet you!", BLUE);
    
    while (1);
    return 0;
}
