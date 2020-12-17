#ifndef __SERVOUTILS_H__
#define __SERVOUTILS_H__

#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <timer.h>
#include <pwm.h>
#include <plic.h>
#include <sysctl.h>
#include <fpioa.h>

#define FORWARD_ANGLE (0)
#define LEFT_ANGLE (90)
#define RIGHT_ANGLE (-90)

#define TIMER_NOR (0)
#define TIMER_CHN (0)
#define TIMER_PWM (1)
#define TIMER_PWM_CHN0 (0)

#define SERVO_FREQ (50)
#define SERVO0_DUTY_MAX (0.09)
#define SERVO0_DUTY_MIN (0.02)

#define SERVOPIN (10)
/**
* Function       servo_move_angle
* @author        jackster
* @date          2020.12.05
* @brief         PWM控制舵机
* @par History   无
*/
/* 
* -90度对应的duty为0.02
* 90度对应的duty为0.09
* 假设占空比线性关系为： duty = a * angle + b
* 求解：
* 0.02 = -90 * a + b
* 0.09 = 90 *a + b
* 得出：
* a = (0.09 - 0.02) / 180
* b = (0.02 + 90 * (0.09 - 0.02) / 180 )
* duty = angle * (0.09 - 0.02) / 180 + (0.09 + 0.02) / 2
*/
double servo_move_angle(pwm_device_number_t pwm_number, pwm_channel_number_t channel, double frequency, int angle)
{
    double duty = angle * (SERVO0_DUTY_MAX - SERVO0_DUTY_MIN) / 180 + (SERVO0_DUTY_MAX + SERVO0_DUTY_MIN) / 2;
    return pwm_set_frequency(pwm_number, channel, frequency, duty);
}

// useage:
// servo_move_angle(TIMER_PWM, TIMER_PWM_CHN0, SERVO_FREQ, -90);

#endif