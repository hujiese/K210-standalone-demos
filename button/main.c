/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        按键控制RGB灯
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 每次按下按键RGB灯亮红色，松开则熄灭。
*/
#include "pin_config.h"
#include "sleep.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "plic.h"

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
    // fpioa映射
    fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
    fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
    fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);

    fpioa_set_function(PIN_KEY, FUNC_KEY);
}

/**
* Function       hardware_init
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
    /* 这里只是为了测试才在中断回调打印数据，正常情况下是不建议这么做的。*/
    printf("IRQ The PIN is %d\n", key_state);

    uint32_t *tmp = (uint32_t *)(ctx);
    printf("count is %d\n", (*tmp)++);

    if (!key_state)
        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_LOW);
    else
        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
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
    /* 设置按键的GPIO模式为上拉输入 */
    gpiohs_set_drive_mode(KEY_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    /* 设置按键的GPIO电平触发模式为上升沿和下降沿 */
    gpiohs_set_pin_edge(KEY_GPIONUM, GPIO_PE_BOTH);
    /* 设置按键GPIO口的中断回调 */
    gpiohs_irq_register(KEY_GPIONUM, 1, key_irq_cb, &g_count);
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
    // 硬件引脚初始化
    hardware_init();

    /* 外部中断初始化 */
    plic_init();
    /* 使能全局中断 */
    sysctl_enable_irq();

    // 初始化RGB灯
    init_rgb();

    // 初始化按键key
    init_key();
    
    while (1);
    return 0;
}
