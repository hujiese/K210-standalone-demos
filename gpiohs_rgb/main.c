/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        高速GPIO点亮RGB灯
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: rgb三种颜色交替点亮，时间间隔为0.5秒。
*/
#include "sleep.h"
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
    // fpioa映射
    fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
    fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
    fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);

}

/**
* Function       hardware_init
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
    // 设置RGB灯的GPIO模式为输出
    gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);

    // 关闭RGB灯
    rgb_all_off();
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
    // RGB灯状态，0=红灯亮，1=绿灯亮，2=蓝灯亮
    int state = 0;

    // 硬件引脚初始化
    hardware_init();
    // 初始化RGB灯
    init_rgb();

    while (1)
    {
        rgb_all_off();   // 先关闭RGB灯
        // 根据state的值点亮不同颜色的灯
        gpiohs_set_pin(state, GPIO_PV_LOW);
        msleep(500);
        state++;
        state = state % 3;
    }
    return 0;
}
