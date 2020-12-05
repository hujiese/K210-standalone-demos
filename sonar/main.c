/**
* @par  Copyright (C): 2016-2022, SCUT
* @file         main.c
* @author       jackster
* @version      V1.0
* @date         2020.05.27
* @brief        测试超声波模块
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 每1s测量一次距离
*/
#include "pin_config.h"
#include "ultrasonic.h"
#include "sleep.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "plic.h"

uint32_t g_count;

/**
* Function       hardware_init
* @author        jackster
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
    fpioa_set_function(PIN_TRIG, FUNC_TRIG);
    fpioa_set_function(PIN_ECHO, FUNC_ECHO);
}

int main(void)
{
    // 硬件引脚初始化
    hardware_init();
    
    while (1)
    {
        long distance = ultrasonic_measure_cm(FUNC_TRIG, FUNC_ECHO, 3000000);
        printf("%ld cm\n", distance);
        sleep(1);
    }
    return 0;
}
