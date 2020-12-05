/**
* @par  Copyright (C): 2016-2022, SCUT
* @file         pin_config.c
* @author       jackster
* @version      V1.0
* @date         2020.12.05
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
#define PIN_TRIG             (12)
#define PIN_ECHO             (13)

/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应
#define TRIG_GPIONUM          (0)
#define ECHO_GPIONUM          (1)

/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口
#define FUNC_TRIG             (FUNC_GPIOHS0 + TRIG_GPIONUM)
#define FUNC_ECHO             (FUNC_GPIOHS0 + ECHO_GPIONUM)

#endif /* _PIN_CONFIG_H_ */
