#ifndef __UTIL_H__
#define __UTIL_H__

#include "net_utils.h"
#include "servo_utils.h"
#include "sonar_utils.h"
#include "esp32_utils.h"
#include "lcd_utils.h"
#include "dvp_camera_utils.h"
#include "yolo_utils.h"
#include "w25qxx.h"

#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL

void io_set_power(void)
{
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

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

void yolo_init(void)
{
    /* yolo 层初始化*/
    lable_init();

// 必须在flash初始化之后才可调用
#if LOAD_KMODEL_FROM_FLASH
    model_data = (uint8_t*)malloc(KMODEL_SIZE + 255);
    uint8_t *model_data_align = (uint8_t*)(((uintptr_t)model_data+255)&(~255));
    w25qxx_read_data(0xA00000, model_data_align, KMODEL_SIZE, W25QXX_QUAD_FAST);
#else
    uint8_t *model_data_align = model_data;
#endif

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
    // sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);

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

     /* flash init */
    w25qxx_init(3, 0);
    w25qxx_enable_quad_mode();

    /* 初始化*/
    yolo_init();
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

void yolo_object_detect(void)
{
    g_dvp_finish_flag = 0;
        
    while (!g_dvp_finish_flag)
        ;
    g_ram_mux ^= 0x01;
    g_dvp_finish_flag = 0;
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
    lcd_draw_picture(0, 0, 320, 240, g_ram_mux ? display_buf_addr1 : display_buf_addr2);

    /* 画识别结果 */
    region_layer_draw_boxes(&obj_detect_rl, drawboxes);

    // g_dvp_finish_flag = 0;
}
#endif