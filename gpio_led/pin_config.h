/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         pin_config.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        硬件引脚与软件GPIO的宏定义
* @details      
* @par History  见如下说明
*                 
* version:	由于K210使用fpioa现场可编程IO阵列，允许用户将255个内部功能映射到芯片外围的48个自由IO上
*           所以把硬件IO和软件GPIO功能抽出来单独设置，这样更容易理解。
*/
#ifndef _PIN_CONFIG_H_
#define _PIN_CONFIG_H_
/*****************************HEAR-FILE************************************/
#include "fpioa.h"

/*****************************HARDWARE-PIN*********************************/
// 硬件IO口，与原理图对应
#define PIN_LED_0             (0)
#define PIN_LED_1             (17)

/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应
#define LED0_GPIONUM          (0)
#define LED1_GPIONUM          (1)

/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口
#define FUNC_LED0             (FUNC_GPIO0 + LED0_GPIONUM)
#define FUNC_LED1             (FUNC_GPIO0 + LED1_GPIONUM)

#endif /* _PIN_CONFIG_H_ */
