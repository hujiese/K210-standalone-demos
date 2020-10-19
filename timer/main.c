/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        定时器控制RGB灯
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 定时器控制。
*/
#include "pin_config.h"
#include "sleep.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "plic.h"
#include "timer.h"

uint32_t g_count;

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
    /* fpioa映射 */
    fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
    fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
    fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);

    fpioa_set_function(PIN_KEY, FUNC_KEY);
}

/**
* Function       rgb_all_off
* @author        Gengyue
* @date          2020.05.27
* @brief         RGB灯灭
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void rgb_all_off(void)
{
    gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
}

/**
* Function       rgb_all_on
* @author        Gengyue
* @date          2020.05.27
* @brief         RGB灯亮白灯
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void rgb_all_on(void)
{
    gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_LOW);
    gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_LOW);
    gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_LOW);
}

/**
* Function       init_rgb
* @author        Gengyue
* @date          2020.05.27
* @brief         初始化RGB灯
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void init_rgb(void)
{
    /* 设置RGB灯的GPIO模式为输出 */
    gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);

    /* 关闭RGB灯 */
    rgb_all_off();
}

/**
* Function       key_irq_cb
* @author        Gengyue
* @date          2020.05.27
* @brief         按键key中断回调函数
* @param[in]     ctx 回调参数
* @param[out]    void
* @retval        0
* @par History   无
*/
int key_irq_cb(void* ctx)
{
    gpio_pin_value_t key_state = gpiohs_get_pin(KEY_GPIONUM);

    if (key_state)
        timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
    else
        timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0);
    return 0;
}

/**
* Function       init_key
* @author        Gengyue
* @date          2020.05.27
* @brief         初始化按键key
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void init_key(void)
{
    // 设置按键的GPIO模式为上拉输入
    gpiohs_set_drive_mode(KEY_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    // 设置按键的GPIO电平触发模式为上升沿和下降沿
    gpiohs_set_pin_edge(KEY_GPIONUM, GPIO_PE_BOTH);
    // 设置按键GPIO口的中断回调
    gpiohs_irq_register(KEY_GPIONUM, 1, key_irq_cb, NULL);

}

/**
* Function       timer_timeout_cb
* @author        Gengyue
* @date          2020.05.27
* @brief         定时器中断回调
* @param[in]     ctx
* @param[out]    void
* @retval        0
* @par History   无
*/
int timer_timeout_cb(void *ctx) {
    uint32_t *tmp = (uint32_t *)(ctx);
    (*tmp)++;
    if ((*tmp)%2)
    {
        rgb_all_on();
    }
    else
    {
        rgb_all_off();
    }
    return 0;
}

/**
* Function       init_timer
* @author        Gengyue
* @date          2020.05.27
* @brief         初始化定时器
* @param[in]     ctx
* @param[out]    void
* @retval        0
* @par History   无
*/
void init_timer(void) {
    /* 定时器初始化 */
    timer_init(TIMER_DEVICE_0);
    /* 设置定时器超时时间，单位为ns */
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 500 * 1e6);
    /* 设置定时器中断回调 */
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, timer_timeout_cb, &g_count);
    /* 使能定时器 */
    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
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

    /* 初始化系统中断并使能 */
    plic_init();
    sysctl_enable_irq();

    /* 初始化RGB灯 */
    init_rgb();

    /* 初始化按键key */
    init_key();

    /* 初始化定时器 */
    init_timer();
    
    while (1);

    return 0;
}
