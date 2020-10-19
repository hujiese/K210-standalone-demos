/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Jackster
* @version      V1.0
* @date         2020.10.04
* @brief        双核实验
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 开启核心1，并且核心0和核心1同时进入while（1）循环，让LED0与LED1交替点亮，时间间隔为1秒
*/
#include <stdio.h>
#include <unistd.h>
#include "gpio.h"
#include "pin_config.h"
#include "bsp.h"
#include "sleep.h"
#include "sysctl.h"

void hardware_init(void)
{
    fpioa_set_function(PIN_LED_0, FUNC_LED0);
    fpioa_set_function(PIN_LED_1, FUNC_LED1);
}

/**
* Function       core1_main
* @author        Jackster
* @date          2020.10.04
* @brief         核心1的主函数
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
int core1_main(void *ctx)
{
    uint64_t core = current_coreid();

    gpio_set_drive_mode(LED1_GPIONUM, GPIO_DM_OUTPUT);

    gpio_pin_value_t value = GPIO_PV_HIGH;

    gpio_set_pin(LED1_GPIONUM, value);

    while(1)
    {
        sleep(2);
        printf("Core %ld is running\n", core);
        gpio_set_pin(LED1_GPIONUM, value);
        value = !value;
    }

    return 0;
}


/**
* Function       main
* @author        Jackster
* @date          2020.10.04
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

    /* 读取当前运行的核心编号 */
    uint64_t core = current_coreid();

    /* 注册核心1，并启动核心1 */
    register_core1(core1_main, NULL);

    gpio_set_drive_mode(LED0_GPIONUM, GPIO_DM_OUTPUT);

    gpio_pin_value_t value = GPIO_PV_HIGH;
    gpio_set_pin(LED0_GPIONUM, value);
    while(1)
    {
        sleep(1);
        printf("Core %ld is running\n", core);
        gpio_set_pin(LED0_GPIONUM, value);
        value = !value;
    }
    return 0;
}
