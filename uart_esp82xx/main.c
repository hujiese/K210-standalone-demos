/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        传输WiFi模块数据
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 开机串口发送数据，接收WiFi模块的数据，经过USB串口到电脑上显示；
*                 接收USB串口的数据，经过WiFi串口发送给WiFi模块。
*/
#include "pin_config.h"
#include "string.h"

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
    /* USB串口 */
    fpioa_set_function(PIN_UART_USB_RX, FUNC_UART_USB_RX);
    fpioa_set_function(PIN_UART_USB_TX, FUNC_UART_USB_TX);

    /* WIFI模块串口 */
    fpioa_set_function(PIN_UART_WIFI_RX, FUNC_UART_WIFI_RX);
    fpioa_set_function(PIN_UART_WIFI_TX, FUNC_UART_WIFI_TX);
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
    /* 硬件初始化 */
    hardware_init();

    // 初始化USB串口，设置波特率为115200
    uart_init(UART_USB_NUM);
    uart_configure(UART_USB_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* 初始化WiFi模块的串口 */
    uart_init(UART_WIFI_NUM);
    uart_configure(UART_WIFI_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* 开机发送hello yahboom! */
    char *hello = {"hello yahboom!\n"};
    uart_send_data(UART_USB_NUM, hello, strlen(hello));

    char recv = 0, send = 0;

    while (1)
    {
        /* 接收WIFI模块的信息 */
        if(uart_receive_data(UART_WIFI_NUM, &recv, 1))
        {
            /* 发送WiFi的数据到USB串口显示 */
            uart_send_data(UART_USB_NUM, &recv, 1);
        }

        /* 接收串口的信息，并发送给WiFi模块 */
        if(uart_receive_data(UART_USB_NUM, &send, 1))
        {
            uart_send_data(UART_WIFI_NUM, &send, 1);
        }
    }
    return 0;
}
