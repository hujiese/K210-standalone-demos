#include <stdio.h>
#include <string.h>
#include "dmac.h"
#include "unistd.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "uarths.h"
#include "bsp.h"
#include "spi.h"
#include "sysctl.h"
#include "pin_config.h"
#include "plic.h"
#include "sleep.h"
#include "esp32_spi_io.h"
#include "esp32_spi.h"

char* WIFI_SSID = "DaGuLion";
char* WIFI_PASSWD = "abc360abc";

// #define SPI_MASTER_CS_LOW()     gpiohs_set_pin(ESP_CS_GPIONUM, GPIO_PV_LOW)
// #define SPI_MASTER_CS_HIGH()    gpiohs_set_pin(ESP_CS_GPIONUM, GPIO_PV_HIGH)

// void _pulseSS() 
// { 
//     SPI_MASTER_CS_HIGH();
//     usleep(5);
//     SPI_MASTER_CS_LOW();
// }

void hardware_init(void)
{
    /* ESP32 */
    fpioa_set_function(PIN_ESP_CS,   FUNC_ESP_SPI_CS);
    fpioa_set_function(PIN_ESP_RST,  FUNC_ESP_SPI_RST);
    fpioa_set_function(PIN_ESP_RDY,  FUNC_ESP_SPI_RDY);
    fpioa_set_function(PIN_ESP_MOSI, FUNC_ESP_SPI_MOSI);
    fpioa_set_function(PIN_ESP_MISO, FUNC_ESP_SPI_MISO);
    fpioa_set_function(PIN_ESP_CLK,  FUNC_ESP_SPI_CLK);

    // gpiohs_set_drive_mode(PIN_ESP_CS, GPIO_DM_OUTPUT);

    /* 使能SPI0和DVP */
    sysctl_set_spi0_dvp_data(1);
}

// const uint8_t CMD_WRITEDATA = 0x02;
// const uint8_t CMD_EMPTY = 0x00;

// static int spi_send_data(uint8_t *data, uint32_t len)
// {
//     _pulseSS();

//     spi_send_data_standard(0, 0, NULL, 0, &CMD_WRITEDATA, 1);
//     spi_send_data_standard(0, 0, NULL, 0, &CMD_EMPTY, 1);
    
//     for (uint8_t i = 0;  i < 32;  ++i)
//          spi_send_data_standard(0, 0, NULL, 0, (const uint8_t *)data, len);

//     _pulseSS();

//     return 0;
// }

int main(void)
{
    /* 硬件引脚初始化 */
    hardware_init();

    /* 设置系统时钟和DVP时钟 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 300000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);

    /* 调试串口初始化 */
    uarths_init();
    esp32_spi_config_io(FUNC_ESP_SPI_CS - FUNC_GPIOHS0, FUNC_ESP_SPI_RST, FUNC_ESP_SPI_RDY - FUNC_GPIOHS0, FUNC_ESP_SPI_MOSI - FUNC_GPIOHS0, FUNC_ESP_SPI_MISO - FUNC_GPIOHS0, FUNC_ESP_SPI_CLK - FUNC_GPIOHS0);

    // esp32_spi_config_io(PIN_ESP_CS_NUM, PIN_ESP_RST_NUM, PIN_ESP_RDY_NUM, PIN_ESP_MOSI_NUM, PIN_ESP_MISO_NUM, PIN_ESP_CLK_NUM);
    esp32_spi_init();
    esp32_spi_connect_AP(WIFI_SSID, WIFI_PASSWD, 3);

    // printf(esp32_spi_is_connected());


    while(1);
    // spi_init(0, SPI_WORK_MODE_0, SPI_FF_STANDARD, 8, 0);
                       
    // spi_set_clk_rate(0, 1000000);

    // uint8_t data[14] = {'A','r','e',' ', 'y','o','u',' ','a','l','i','v','e','?'};
    // while (1)
    // {
    //     spi_send_data(data, strlen(data));
    //     printf("send ok ....\n");
    //     sleep(1);
    // }
    
    return 0;
}