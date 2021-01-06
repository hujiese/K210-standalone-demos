/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include "fpioa.h"
#include <string.h>
#include "uart.h"
#include "gpiohs.h"
#include "sysctl.h"

#define UART_NUM    UART_DEVICE_3

int on_uart_send(void *ctx)
{
    // uint8_t v_uart = *((uint32_t *)ctx) + 1 + 0x30;
    // uart_irq_unregister(UART_NUM, UART_SEND);
    // char *v_send_ok = "Send ok Uart: ";
    // uart_send_data(UART_NUM, v_send_ok,strlen(v_send_ok));
    // uart_send_data(UART_NUM, (char *)&v_uart,1);
    return 0;
}

int on_uart_recv(void *ctx)
{
    char v_buf[8];
    int ret =  uart_receive_data(UART_NUM, v_buf, 8);
    char *v_send_ok = "Send ok Uart: ";
    // printf("receive %s\n", v_buf);
    uart_send_data(UART_NUM, v_send_ok,strlen(v_send_ok));
    uart_send_data(UART_NUM, v_buf, ret);
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

    uart_set_receive_trigger(UART_NUM, UART_RECEIVE_FIFO_8);
    uart_irq_register(UART_NUM, UART_RECEIVE, on_uart_recv, NULL, 2);

    uart_set_send_trigger(UART_NUM, UART_SEND_FIFO_0);
    uint32_t v_uart_num = UART_NUM;
    uart_irq_register(UART_NUM, UART_SEND, on_uart_send, &v_uart_num, 2);

    char *hel = {"hello world!\n"};
    uart_send_data(UART_NUM, hel, strlen(hel));

    while(1);
}

