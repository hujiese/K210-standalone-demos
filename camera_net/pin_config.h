/**
* @par  Copyright (C): 2016-2022, SCUT
* @file         pin_config.c
* @author       jackster
* @version      V1.0
* @date         2020.10.19
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
#define PIN_LCD_CS             (36)
#define PIN_LCD_RST            (37)
#define PIN_LCD_RS             (38)
#define PIN_LCD_WR             (39)

// camera
#define PIN_DVP_PCLK           (47)
#define PIN_DVP_XCLK           (46)
#define PIN_DVP_HSYNC          (45)
#define PIN_DVP_PWDN           (44)
#define PIN_DVP_VSYNC          (43)
#define PIN_DVP_RST            (42)
#define PIN_DVP_SCL            (41)
#define PIN_DVP_SDA            (40)

// 按键
#define PIN_KEY               (16)

/***************************************
 *      K210_GPIO    |       ESP32
 * ===================================
 *          IO25     |        IO5
 *          IO27     |        IO18
 *          IO26     |        IO23
 *          IO28     |        IO14
 *          IO8      |        RST
 *          IO9      |        IO25
 *          5V       |        5V
 *          GND      |        GND
/***************************************/


// ESP32 SPI
#define PIN_ESP_CS              (25)
#define PIN_ESP_RST             (8)
#define PIN_ESP_RDY             (9)
#define PIN_ESP_MOSI            (28)    
#define PIN_ESP_MISO            (26)
#define PIN_ESP_CLK             (27)

/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应
#define LCD_RST_GPIONUM        (0)
#define LCD_RS_GPIONUM         (1)

#define KEY_GPIONUM            (6)

#define TF_CS_GPIONUM          (2)
/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口
#define FUNC_LCD_CS             (FUNC_SPI0_SS3)
#define FUNC_LCD_RST            (FUNC_GPIOHS0 + LCD_RST_GPIONUM)
#define FUNC_LCD_RS             (FUNC_GPIOHS0 + LCD_RS_GPIONUM)
#define FUNC_LCD_WR             (FUNC_SPI0_SCLK)

#define FUNC_KEY                (FUNC_GPIOHS0 + KEY_GPIONUM)

#define FUNC_ESP_SPI_CS          (FUNC_GPIOHS10)
#define FUNC_ESP_SPI_RST         (FUNC_GPIOHS11)
#define FUNC_ESP_SPI_RDY         (FUNC_GPIOHS12)
#define FUNC_ESP_SPI_MOSI        (FUNC_GPIOHS13)
#define FUNC_ESP_SPI_MISO        (FUNC_GPIOHS14)
#define FUNC_ESP_SPI_CLK         (FUNC_GPIOHS15)

#endif /* _PIN_CONFIG_H_ */
