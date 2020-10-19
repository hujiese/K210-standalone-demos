/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        双核实验
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 开启核心1，并且核心0和核心1同时进入while（1）循环，并打印不同数据
*/
#include <stdio.h>
#include "bsp.h"
#include "sleep.h"
#include "sysctl.h"

/**
* Function       core1_main
* @author        Gengyue
* @date          2020.05.27
* @brief         核心1的主函数
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
int core1_main(void *ctx)
{
    int state = 1;
    uint64_t core = current_coreid();
    printf("Core %ld say: Hello world\n", core);

    while(1)
    {
        msleep(500);
        if (state = !state)
        {
            printf("Core %ld is running too!\n", core);
        }
        else
        {
            printf("Core %ld is running faster!\n", core);
        }
    }
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
    /* 读取当前运行的核心编号 */
    uint64_t core = current_coreid();
    printf("Core %ld say: Hello yahboom\n", core);
    /* 注册核心1，并启动核心1 */
    register_core1(core1_main, NULL);

    while(1)
    {
        sleep(1);
        printf("Core %ld is running\n", core);
    }
    return 0;
}
