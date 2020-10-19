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
#define PIN_SPK_WS             (30)
#define PIN_SPK_DATA           (31)
#define PIN_SPK_BCK            (32)

/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应


/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口
#define FUNC_SPK_WS            FUNC_I2S2_WS
#define FUNC_SPK_DATA          FUNC_I2S2_OUT_D0
#define FUNC_SPK_BCK           FUNC_I2S2_SCLK

#endif /* _PIN_CONFIG_H_ */
