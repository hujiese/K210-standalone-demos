/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        直接内存存取控制器DMAC
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 串口通过DMA通道传输数据，并控制RGB灯颜色.
*                 串口接收以下数据对应的功能：
*                 FFAA11 ---- 红灯亮
*                 FFAA22 ---- 红灯灭
*                 FFAA33 ---- 绿灯亮
*                 FFAA44 ---- 绿灯灭
*                 FFAA55 ---- 蓝灯亮
*                 FFAA66 ---- 蓝灯灭
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "uart.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "pin_config.h"


#define CMD_RGB_R_ON              0x11
#define CMD_RGB_R_OFF             0x22
#define CMD_RGB_G_ON              0x33
#define CMD_RGB_G_OFF             0x44
#define CMD_RGB_B_ON              0x55
#define CMD_RGB_B_OFF             0x66

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
    /* fpioa映射 */
    fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
    fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
    fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);

    fpioa_set_function(PIN_UART_USB_RX, FUNC_UART_USB_RX);
    fpioa_set_function(PIN_UART_USB_TX, FUNC_UART_USB_TX);
}

/**
* Function       rgb_all_off
* @author        Gengyue
* @date          2020.05.27
* @brief         RGB灯灭
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void rgb_all_off(void)
{
    gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
}

/**
* Function       init_rgb
* @author        Gengyue
* @date          2020.05.27
* @brief         初始化RGB灯
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void init_rgb(void)
{
    /* 设置RGB灯的GPIO模式为输出 */
    gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);

    /* 关闭RGB灯 */
    rgb_all_off();
}

/**
* Function       parse_cmd
* @author        Gengyue
* @date          2020.05.27
* @brief         解析接收到的数据
* @param[in]     cmd: 接收的命令
* @param[out]    void
* @retval        0
* @par History   无
*/
int parse_cmd(uint8_t *cmd)
{
    switch(*cmd)
    {
    case CMD_RGB_R_ON:
        /* RGB亮红灯*/
        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_LOW);
        break;
    case CMD_RGB_R_OFF:
        /* RGB红灯灭*/
        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
        break;
    case CMD_RGB_G_ON:
        /* RGB亮绿灯*/
        gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_LOW);
        break;
    case CMD_RGB_G_OFF:
        /* RGB绿灯灭*/
        gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
        break;
    case CMD_RGB_B_ON:
        /* RGB亮蓝灯*/
        gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_LOW);
        break;
    case CMD_RGB_B_OFF:
        /* RGB蓝灯灭*/
        gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
        break;
    }
    return 0;
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
    /* 硬件引脚初始化 */
    hardware_init();

    /* 初始化RGB灯 */
    init_rgb();

    /* 初始化串口，设置波特率为115200 */
    uart_init(UART_USB_NUM);
    uart_configure(UART_USB_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* 开机发送hello yahboom!欢迎语 */
    char *hel = {"hello yahboom!\n"};
    uart_send_data_dma(UART_USB_NUM, DMAC_CHANNEL0, (uint8_t *)hel, strlen(hel));

    uint8_t recv = 0;
    int rec_flag = 0;

    while (1)
    {
        /* 通过DMA通道1接收串口数据，保存到recv中 */
        uart_receive_data_dma(UART_USB_NUM, DMAC_CHANNEL1, &recv, 1);
        /* 以下是判断协议，必须是FFAA开头的数据才可以 */
        switch(rec_flag)
        {
        case 0:
            if(recv == 0xFF)
                rec_flag = 1;
            break;
        case 1:
            if(recv == 0xAA)
                rec_flag = 2;
            else if(recv != 0xFF)
                rec_flag = 0;
            break;
        case 2:
            /* 解析真正的数据 */
            parse_cmd(&recv);
            /* 通过dma通道0发送串口数据 */
            uart_send_data_dma(UART_USB_NUM, DMAC_CHANNEL0, &recv, 1);
            rec_flag = 0;
            break;
        }
    }
    return 0;
}
