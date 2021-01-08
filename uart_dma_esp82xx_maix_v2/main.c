#include <stdio.h>
#include "fpioa.h"
#include <string.h>
#include "uart.h"
#include "gpiohs.h"
#include "sysctl.h"
#include <unistd.h>
#include <stdlib.h>
#include "pin_config.h"

#define BUFFSIZE 8

int on_uart_recv(void *ctx)
{
    uint8_t* v_buf = (uint8_t *)ctx;
    printf("%s", v_buf);
    memset(v_buf, 0, BUFFSIZE);
    // uart_receive_data_dma_irq 函数只能被触发一次，所以需要重新注册中断函数
    uart_receive_data_dma_irq(UART_WIFI_NUM, DMAC_CHANNEL3, v_buf, BUFFSIZE, on_uart_recv, ctx, 1);
    return 0;
}

void io_mux_init(void)
{
    fpioa_set_function(PIN_UART_WIFI_RX, FUNC_UART_WIFI_RX);
    fpioa_set_function(PIN_UART_WIFI_TX, FUNC_UART_WIFI_TX);
}

int main()
{
    io_mux_init();
    plic_init();
    sysctl_enable_irq();

    uart_init(UART_WIFI_NUM);
    uart_configure(UART_WIFI_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    uint8_t v_buf[BUFFSIZE];
    memset(v_buf, 0, BUFFSIZE);
    // uart_set_receive_trigger(UART_WIFI_NUM, UART_RECEIVE_FIFO_1);
    uart_receive_data_dma_irq(UART_WIFI_NUM, DMAC_CHANNEL3, v_buf, BUFFSIZE, on_uart_recv, v_buf, 1);

    while(1)
    {
        // uart_send_data_dma_irq(UART_WIFI_NUM, DMAC_CHANNEL4, msg, strlen(msg), NULL, NULL, 2);
        // sleep(2);
        // printf("-----%s", v_buf);
    }
}

