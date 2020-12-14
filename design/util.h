#ifndef __UTIL_H__
#define __UTIL_H__

#include "net_utils.h"
#include "servo_utils.h"
#include "sonar_utils.h"
#include "esp32_utils.h"
#include "lcd_utils.h"
#include "dvp_camera_utils.h"

void esp32_config(void)
{
    /* ESP32 */
    fpioa_set_function(PIN_ESP_CS,   FUNC_ESP_SPI_CS);
    fpioa_set_function(PIN_ESP_RST,  FUNC_ESP_SPI_RST);
    fpioa_set_function(PIN_ESP_RDY,  FUNC_ESP_SPI_RDY);
    fpioa_set_function(PIN_ESP_MOSI, FUNC_ESP_SPI_MOSI);
    fpioa_set_function(PIN_ESP_MISO, FUNC_ESP_SPI_MISO);
    fpioa_set_function(PIN_ESP_CLK,  FUNC_ESP_SPI_CLK);

}

void lcd_config(void)
{
    /* lcd */
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);

    g_ram_mux = 0;
}

void dvp_camera_config(void)
{
    /* DVP camera */
    fpioa_set_function(PIN_DVP_RST,   FUNC_CMOS_RST);
    fpioa_set_function(PIN_DVP_PWDN,  FUNC_CMOS_PWDN);
    fpioa_set_function(PIN_DVP_XCLK,  FUNC_CMOS_XCLK);
    fpioa_set_function(PIN_DVP_VSYNC, FUNC_CMOS_VSYNC);
    fpioa_set_function(PIN_DVP_HSYNC, FUNC_CMOS_HREF);
    fpioa_set_function(PIN_DVP_PCLK,  FUNC_CMOS_PCLK);
    fpioa_set_function(PIN_DVP_SCL,   FUNC_SCCB_SCLK);
    fpioa_set_function(PIN_DVP_SDA,   FUNC_SCCB_SDA);

    /* 设置拍照flag为0 */
    g_save_flag = 0;
}

void sonar_config(void)
{
    // fpioa映射
    fpioa_set_function(PIN_TRIG, FUNC_TRIG);
    fpioa_set_function(PIN_ECHO, FUNC_ECHO);
}

void servo_config(void)
{
        /* Init FPIOA pin mapping */
    fpioa_set_function(SERVOPIN, FUNC_TIMER1_TOGGLE1);
    // 中断初始化在主程序中完成
    // /* Init Platform-Level Interrupt Controller(PLIC) */
    plic_init();
    // /* Enable global interrupt for machine mode of RISC-V */
    sysctl_enable_irq();
    /* Init timer */
    timer_init(TIMER_NOR);
    /* Set timer interval to 10ms */         
    timer_set_interval(TIMER_NOR, TIMER_CHN, 10000000);
    /* Set timer callback function with repeat method */
    timer_irq_register(TIMER_NOR, TIMER_CHN, 0, 1, NULL, NULL);
    /* Enable timer */
    timer_set_enable(TIMER_NOR, TIMER_CHN, 1);
    /* Init PWM */
    pwm_init(TIMER_PWM);
    /* Set PWM to 50Hz */
    servo_move_angle(TIMER_PWM, TIMER_PWM_CHN0, SERVO_FREQ, 0);
    pwm_set_enable(TIMER_PWM, TIMER_PWM_CHN0, 1);
}

void io_set_power(void)
{
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

void hardware_init(void)
{
    /* 一些硬件管脚初始化 */
    lcd_config();
    dvp_camera_config();
    sonar_config();
    esp32_config();
    servo_config();
    sysctl_set_spi0_dvp_data(1);

    esp32_spi_config_io(FUNC_ESP_SPI_CS - FUNC_GPIOHS0, FUNC_ESP_SPI_RST, FUNC_ESP_SPI_RDY - FUNC_GPIOHS0, FUNC_ESP_SPI_MOSI - FUNC_GPIOHS0, FUNC_ESP_SPI_MISO - FUNC_GPIOHS0, FUNC_ESP_SPI_CLK - FUNC_GPIOHS0);

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

    /* 初始化摄像头 */
    ov2640_init();

    /* 初始化esp32 */
    esp32_spi_init();

}

void capture(void)
{
    /* 等待摄像头采集结束，然后清除结束标志 */
    while (g_dvp_finish_flag == 0)
        ;
    g_ram_mux ^= 0x01;
    g_dvp_finish_flag = 0;

    /* 显示画面 */
    lcd_draw_picture(0, 0, 320, 240, g_ram_mux ? display_buf_addr1 : display_buf_addr2);
}
#endif