#ifndef _PIN_CONFIG_H_
#define _PIN_CONFIG_H_

#include "fpioa.h"

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


#define PIN_ESP_CS              (25)
#define PIN_ESP_RST             (8)
#define PIN_ESP_RDY             (9)
#define PIN_ESP_MOSI            (28)    
#define PIN_ESP_MISO            (26)
#define PIN_ESP_CLK             (27)

#define FUNC_ESP_SPI_CS          (FUNC_GPIOHS10)
#define FUNC_ESP_SPI_RST         (FUNC_GPIOHS11)
#define FUNC_ESP_SPI_RDY         (FUNC_GPIOHS12)
#define FUNC_ESP_SPI_MOSI        (FUNC_GPIOHS13)
#define FUNC_ESP_SPI_MISO        (FUNC_GPIOHS14)
#define FUNC_ESP_SPI_CLK         (FUNC_GPIOHS15)

// #define PIN_ESP_CS_NUM              (10)
// #define PIN_ESP_RST_NUM             (11)
// #define PIN_ESP_RDY_NUM             (12)
// #define PIN_ESP_MOSI_NUM            (13)    
// #define PIN_ESP_MISO_NUM            (14)
// #define PIN_ESP_CLK_NUM             (15)

// #define FUNC_ESP_SPI_CS          (FUNC_GPIOHS0 + PIN_ESP_CS_NUM)
// #define FUNC_ESP_SPI_RST         (FUNC_GPIOHS0 + PIN_ESP_RST_NUM)
// #define FUNC_ESP_SPI_RDY         (FUNC_GPIOHS0 + PIN_ESP_RDY_NUM)
// #define FUNC_ESP_SPI_MOSI        (FUNC_GPIOHS0 + PIN_ESP_MOSI_NUM)
// #define FUNC_ESP_SPI_MISO        (FUNC_GPIOHS0 + PIN_ESP_MISO_NUM)
// #define FUNC_ESP_SPI_CLK         (FUNC_GPIOHS0 + PIN_ESP_CLK_NUM)



#endif /* _PIN_CONFIG_H_ */