/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        FPIOA映射和GPIO驱动LED灯
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: LED0与LED1交替点亮，时间间隔为1秒。
*/
#include <stdio.h>
#include <unistd.h>
#include "gpio.h"
#include "pin_config.h"

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
    fpioa_set_function(PIN_LED_0, FUNC_LED0);
    fpioa_set_function(PIN_LED_1, FUNC_LED1);
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
    hardware_init();// 硬件引脚初始化

    gpio_init();    // 使能GPIO的时钟
    
    // 设置LED0和LED1的GPIO模式为输出
    gpio_set_drive_mode(LED0_GPIONUM, GPIO_DM_OUTPUT);
    gpio_set_drive_mode(LED1_GPIONUM, GPIO_DM_OUTPUT);
    
    // 先关闭LED0和LED1
    gpio_pin_value_t value = GPIO_PV_HIGH;
    gpio_set_pin(LED0_GPIONUM, value);
    gpio_set_pin(LED1_GPIONUM, value);

    while (1)
    {
        sleep(1);
        gpio_set_pin(LED0_GPIONUM, value);
        gpio_set_pin(LED1_GPIONUM, value = !value);
    }
    return 0;
}
