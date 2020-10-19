/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        串口实验
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 开机串口发送数据，等待接收数据并发送接收到的数据.
*/
#include <string.h>
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
    // fpioa映射
    fpioa_set_function(PIN_UART_USB_RX, FUNC_UART_USB_RX);
    fpioa_set_function(PIN_UART_USB_TX, FUNC_UART_USB_TX);
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
    hardware_init();
    // 初始化串口3，设置波特率为115200
    uart_init(UART_USB_NUM);
    uart_configure(UART_USB_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* Hello world! */
    char *hello = {"Hello world!\n"};
    uart_send_data(UART_USB_NUM, hello, strlen(hello));

    char recv = 0;

    while (1)
    {
        /* 等待串口信息，并通过串口发送出去 */
        while(uart_receive_data(UART_USB_NUM, &recv, 1))
        {
            uart_send_data(UART_USB_NUM, &recv, 1);
        }
    }
    return 0;
}
