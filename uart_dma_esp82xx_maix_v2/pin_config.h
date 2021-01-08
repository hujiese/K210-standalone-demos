#ifndef _PIN_CONFIG_H_
#define _PIN_CONFIG_H_
/*****************************HEAR-FILE************************************/
#include "fpioa.h"
#include "uart.h"

/*****************************HARDWARE-PIN*********************************/
// 硬件IO口，与原理图对应

#define PIN_UART_WIFI_RX      (4) // 电路上接ESP的TXD
#define PIN_UART_WIFI_TX      (5) // 电路上接ESP的RXD

/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应

#define UART_WIFI_NUM          UART_DEVICE_3

/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口

#define FUNC_UART_WIFI_RX      (FUNC_UART1_RX + UART_WIFI_NUM * 2)
#define FUNC_UART_WIFI_TX      (FUNC_UART1_TX + UART_WIFI_NUM * 2)

#endif /* _PIN_CONFIG_H_ */
