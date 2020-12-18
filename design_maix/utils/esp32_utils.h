#ifndef __ESP32UTILS_H__
#define __ESP32UTILS_H__

#include "fpioa.h"
#include "esp32_spi_io.h"
#include "esp32_spi.h"

/*****************************HARDWARE-PIN*********************************/
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

/*****************************FUNC-GPIO************************************/

#define FUNC_ESP_SPI_CS          (FUNC_GPIOHS10)
#define FUNC_ESP_SPI_RST         (FUNC_GPIOHS11)
#define FUNC_ESP_SPI_RDY         (FUNC_GPIOHS12)
#define FUNC_ESP_SPI_MOSI        (FUNC_GPIOHS13)
#define FUNC_ESP_SPI_MISO        (FUNC_GPIOHS14)
#define FUNC_ESP_SPI_CLK         (FUNC_GPIOHS15)

#endif