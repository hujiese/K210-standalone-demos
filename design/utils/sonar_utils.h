#ifndef __SONARUTILS_H__
#define __SONARUTILS_H__

/*****************************HEAR-FILE************************************/
#include "fpioa.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "plic.h"
#include "ultrasonic.h"
/*****************************HARDWARE-PIN*********************************/
// 硬件IO口，与原理图对应
#define PIN_TRIG             (12)
#define PIN_ECHO             (13)

/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应
#define TRIG_GPIONUM          (2)
#define ECHO_GPIONUM          (3)

/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口
#define FUNC_TRIG             (FUNC_GPIOHS0 + TRIG_GPIONUM)
#define FUNC_ECHO             (FUNC_GPIOHS0 + ECHO_GPIONUM)

// useage:
// long distance = ultrasonic_measure_cm(FUNC_TRIG, FUNC_ECHO, 3000000);
// printf("%ld cm\n", distance);

#endif