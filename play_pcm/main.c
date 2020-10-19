/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        扬声器播放声音
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 扬声器播放一次PCM数据的声音，要重新播放请按复位键。
*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "i2s.h"
#include "sysctl.h"
#include "fpioa.h"
#include "uarths.h"
#include "pcm.h"
#include "gpiohs.h"
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
    /*
    ** SPK_WS---IO30
    ** SPK_DATA-IO31
    ** SPK_BCK--IO32
    */
    fpioa_set_function(PIN_SPK_WS, FUNC_SPK_WS);
    fpioa_set_function(PIN_SPK_DATA, FUNC_SPK_DATA);
    fpioa_set_function(PIN_SPK_BCK, FUNC_SPK_BCK);
}

uint8_t state = 1;

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

    /* 设置系统时钟 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 320000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

    /* 初始化I2S，第三个参数为设置通道掩码，通道0:0x03,通道1：0x0C,通道2：0x30,通道3:0xC0 */
    i2s_init(I2S_DEVICE_2, I2S_TRANSMITTER, 0x03);

    /* 设置I2S发送数据的通道参数 */
    i2s_tx_channel_config(
        I2S_DEVICE_2, /* I2S设备号*/
        I2S_CHANNEL_0, /* I2S通道 */
        RESOLUTION_16_BIT, /* 接收数据位数 */
        SCLK_CYCLES_32, /* 单个数据时钟数 */
        TRIGGER_LEVEL_4, /* DMA触发时FIFO深度 */
        RIGHT_JUSTIFYING_MODE); /* 工作模式 */


    /* 播放一次音乐，如果需要重新播放请按复位键 */
    i2s_play(
        I2S_DEVICE_2, /* I2S设备号 */
        DMAC_CHANNEL0, /* DMA通道号 */ 
        (uint8_t *)test_pcm, /* 播放的PCM数据 */
        sizeof(test_pcm), /* PCM数据的长度 */
        1024, /* 单次发送数量 */
        16, /* 单次采样位宽 */
        2); /* 声道数 */

    while (1);
    return 0;
}
