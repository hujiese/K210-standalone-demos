#include <stdio.h>
#include "fpioa.h"
#include <string.h>
#include "uart.h"
#include "gpiohs.h"
#include "sysctl.h"
#include <unistd.h>
#include <stdlib.h>

#define UART_NUM    UART_DEVICE_3

int on_uart_send(void *ctx)
{
    // char* v_buf = (char *)ctx;
    // uart_send_data_dma_irq(UART_NUM, DMAC_CHANNEL4, v_buf, strlen(v_buf), on_uart_send, v_buf, 2);
    return 0;
}

int on_uart_recv(void *ctx)
{
    char* v_buf = (char *)ctx;
    printf("receive %s\n", v_buf);
    char *v_send_ok = "Send ok Uart: ";
    uart_send_data(UART_NUM, v_send_ok,strlen(v_send_ok));
    uart_send_data(UART_NUM, v_buf, strlen(v_buf));

    // uart_receive_data_dma_irq 函数只能被触发一次，所以需要重新注册中断函数
    uart_receive_data_dma_irq(UART_NUM, DMAC_CHANNEL3, v_buf, 8, on_uart_recv, v_buf, 2);
    return 0;
}

void io_mux_init(void)
{
    fpioa_set_function(4, FUNC_UART1_RX + UART_NUM * 2);
    fpioa_set_function(5, FUNC_UART1_TX + UART_NUM * 2);
}

int main()
{
    io_mux_init();
    plic_init();
    sysctl_enable_irq();

    uart_init(UART_NUM);
    uart_configure(UART_NUM, 115200, 8, UART_STOP_1, UART_PARITY_NONE);

    char v_buf[8];
    uart_receive_data_dma_irq(UART_NUM, DMAC_CHANNEL3, v_buf, 8, on_uart_recv, v_buf, 2);

    char* msg = "Hello !\n";

    while(1)
    {
        uart_send_data_dma_irq(UART_NUM, DMAC_CHANNEL4, msg, strlen(msg), on_uart_send, msg, 2);
        sleep(2);

    }
}

