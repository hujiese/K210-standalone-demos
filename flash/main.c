/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        flash实验
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 先向flash写入数据，然后读取出来,对比写入和读取的数据是否一致，
*                 如果不一致则打印错误信息。
*/
#include <stdio.h>
#include "fpioa.h"
#include "sysctl.h"
#include "w25qxx.h"
#include "uarths.h"
#include "spi.h"

#define BUF_LENGTH (40 * 1024 + 5)
#define DATA_ADDRESS 0xB00000

uint8_t write_buf[BUF_LENGTH];
uint8_t read_buf[BUF_LENGTH];

/**
* Function       flash_init
* @author        Gengyue
* @date          2020.05.27
* @brief         flash初始化
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
int flash_init(void)
{
    uint8_t manuf_id, device_id;
    uint8_t spi_index = 3, spi_ss = 0;
    printf("flash init \n");

    w25qxx_init(spi_index, spi_ss, 60000000);
    /* 读取flash的ID */
    w25qxx_read_id(&manuf_id, &device_id);
    printf("manuf_id:0x%02x, device_id:0x%02x\n", manuf_id, device_id);
    if ((manuf_id != 0xEF && manuf_id != 0xC8) || (device_id != 0x17 && device_id != 0x16))
    {
        /* flash初始化失败 */
        printf("w25qxx_read_id error\n");
        printf("manuf_id:0x%02x, device_id:0x%02x\n", manuf_id, device_id);
        return 0;
    }
    else
    {
        return 1;
    } 
}

/**
* Function       flash_write_data
* @author        Gengyue
* @date          2020.05.27
* @brief         flash写入数据
* @param[in]     data_buf
* @param[in]     length
* @param[out]    void
* @retval        void
* @par History   无
*/
void flash_write_data(uint8_t *data_buf, uint32_t length)
{
    uint64_t start = sysctl_get_time_us();
    /* flash写入数据 */
    w25qxx_write_data(DATA_ADDRESS, data_buf, length);
    uint64_t stop = sysctl_get_time_us();
    /* 打印写入数据的时间（us） */
    printf("write data finish:%ld us\n", (stop - start));
}

/**
* Function       flash_read_data
* @author        Gengyue
* @date          2020.05.27
* @brief         flash读取数据
* @param[in]     data_buf
* @param[in]     length
* @param[out]    void
* @retval        void
* @par History   无
*/
void flash_read_data(uint8_t *data_buf, uint32_t length)
{
    uint64_t start = sysctl_get_time_us();
    /* flash读取数据 */
    w25qxx_read_data(DATA_ADDRESS, data_buf, length);
    uint64_t stop = sysctl_get_time_us();
    /* 打印读取数据的时间（us） */
    printf("read data finish:%ld us\n", (stop - start));
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
    /* 设置新PLL0频率 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    uarths_init();
    
    /* 初始化flash */
    uint8_t res = 0;
    res = flash_init();
    if (res == 0) return 0;

    /* 给缓存写入的数据赋值 */
    for (int i = 0; i < BUF_LENGTH; i++)
        write_buf[i] = (uint8_t)(i);
    
    /* 清空读取的缓存数据 */
    for(int i = 0; i < BUF_LENGTH; i++)
        read_buf[i] = 0;

    printf("flash start write data\n");

    /* flash写入数据 */
    flash_write_data(write_buf, BUF_LENGTH);

    /*flash读取数据*/
    flash_read_data(read_buf, BUF_LENGTH);

    /* 比较数据，如果有不同则打印错误信息 */
    for (int i = 0; i < BUF_LENGTH; i++)
    {
        if (read_buf[i] != write_buf[i])
        {
            printf("flash read error\n");
            return 0;
        }      
    }
    printf("spi3 flash master test ok\n");
    while (1)
        ;
    return 0;
}
