/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        PWM输出显示呼吸灯效果
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: PWM输出显示呼吸灯效果。
*/
#include <stdio.h>
#include <unistd.h>
#include "gpio.h"
#include "timer.h"
#include "pwm.h"
#include "sysctl.h"
#include "plic.h"
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

    fpioa_set_function(PIN_RGB_R, FUNC_TIMER2_TOGGLE2);
}

/**
* Function       timer_timeout_cb
* @author        Gengyue
* @date          2020.05.27
* @brief         定时器中断回调
* @param[in]     ctx
* @param[out]    void
* @retval        0
* @par History   无
*/
int timer_timeout_cb(void *ctx) {
    static double duty_cycle = 0.01;
    /* 0为渐增，1为渐减 */
    static int flag = 0;      

    /* 传入cycle的不同值，调节PWM的占用比，也就是调节灯的亮度 */
    pwm_set_frequency(PWM_DEVICE_2, PWM_CHANNEL_1, 200000, duty_cycle);

    /* 修改cycle的值，让其在区间（0,1）内渐增和渐减 */
    flag ? (duty_cycle -= 0.01): (duty_cycle += 0.01);
    if(duty_cycle > 1.0)
    {
        duty_cycle = 1.0;
        flag = 1;
    }
    else if (duty_cycle < 0.0)
    {
        duty_cycle = 0.0;
        flag = 0;
    }
    return 0;
}

/**
* Function       init_timer
* @author        Gengyue
* @date          2020.05.27
* @brief         初始化定时器
* @param[in]     ctx
* @param[out]    void
* @retval        0
* @par History   无
*/
void init_timer(void) {
    /* 定时器初始化 */
    timer_init(TIMER_DEVICE_0);
    /* 设置定时器超时时间，单位为ns */
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 10 * 1e6);
    /* 设置定时器中断回调 */
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, timer_timeout_cb, NULL);
    /* 使能定时器 */
    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
}

/**
* Function       init_pwm
* @author        Gengyue
* @date          2020.05.27
* @brief         初始化PWM
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/

// FUNC_TIMER1_TOGGLE1  ->  PWM_CHANNEL_0
// FUNC_TIMER1_TOGGLE2  ->  PWM_CHANNEL_1
// FUNC_TIMER1_TOGGLE3  ->  PWM_CHANNEL_2
// FUNC_TIMER1_TOGGLE4  ->  PWM_CHANNEL_3

// FUNC_TIMERx_ -> PWM_DEVICE_x

void init_pwm(void)
{
    /* 初始化PWM */
    pwm_init(PWM_DEVICE_2);
    /* 设置PWM频率为200KHZ，占空比为0.5的方波 */
    pwm_set_frequency(PWM_DEVICE_2, PWM_CHANNEL_1, 200000, 0.5);
    /* 使能 PWM 输出 */
    pwm_set_enable(PWM_DEVICE_2, PWM_CHANNEL_1, 1);
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

    /* 系统中断初始化和使能 */
    plic_init();
    sysctl_enable_irq();
    
    /* 初始化定时器 */
    init_timer();

    /* 初始化PWM */
    init_pwm();

    while(1);

    return 0;
}
