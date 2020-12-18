#include "ultrasonic.h"
#include "fpioa.h"
#include "gpio.h"
#include "gpiohs.h"
#include "sleep.h"
#include "sysctl.h"
#include "printf.h"

#define time_us()  (unsigned long)(read_csr(mcycle)/(sysctl_clock_get_freq(SYSCTL_CLOCK_CPU)/1000000))

typedef void (*set_pin_func_t)(uint8_t gpio, gpio_pin_value_t value);
typedef gpio_pin_value_t (*get_pin_func_t)(uint8_t gpio);
typedef void (*set_mode_func_t)(uint8_t gpio, gpio_drive_mode_t mode);

static long ultrasonic_measure_us(uint8_t trig, uint8_t echo, uint32_t timeout_us)
{
    unsigned long time_start;
    unsigned long time_pule_start;
    unsigned long time_pule_end;
    set_pin_func_t set_pin = NULL;
    get_pin_func_t get_pin = NULL;
    set_mode_func_t set_mode = NULL;

    if( fpioa_get_io_by_function(trig) < 0 || fpioa_get_io_by_function(echo) < 0 )
        return -2;
    if((trig >= FUNC_GPIO0 && trig <= FUNC_GPIO7) && (echo >= FUNC_GPIO0 && echo <= FUNC_GPIO7))
    {
        set_mode = gpio_set_drive_mode;
        set_pin = gpio_set_pin;
        get_pin = gpio_get_pin;
        trig -= FUNC_GPIO0;
        echo -= FUNC_GPIO0;
    }
    else if((trig >= FUNC_GPIOHS0 && trig <= FUNC_GPIOHS31) && (echo >= FUNC_GPIOHS0 && echo <= FUNC_GPIOHS31))
    {
        set_mode = gpiohs_set_drive_mode;
        set_pin = gpiohs_set_pin;
        get_pin = gpiohs_get_pin;
        trig -= FUNC_GPIOHS0;
        echo -= FUNC_GPIOHS0;
    }
    else
    {
        return -1;
    }
    set_mode(trig, GPIO_DM_OUTPUT);
    set_pin(trig, GPIO_PV_LOW);
    usleep(2);
    set_pin(trig, GPIO_PV_HIGH);
    usleep(5);
    set_pin(trig, GPIO_PV_LOW);
    
    set_mode(echo, GPIO_DM_INPUT);
    time_start = time_us();

    while(get_pin(echo)==GPIO_PV_HIGH)
    {
        if(time_us() - time_start >= timeout_us)
            return 0;
    }
    while(get_pin(echo) == GPIO_PV_LOW)
    {
        if(time_us() - time_start >= timeout_us)
            return 0;
    }
    time_pule_start = time_us();
    while(get_pin(echo) == GPIO_PV_HIGH)
    {
        if(time_us() - time_start >= timeout_us)
            return 0;
    }
    time_pule_end = time_us();
    return (long)(time_pule_end - time_pule_start);
}

long ultrasonic_measure_cm(uint8_t trig_fun, uint8_t echo_fun, uint32_t timeout_us)
{
    long us = ultrasonic_measure_us(trig_fun, echo_fun, timeout_us);
    if(us > 0)
        return us/29/2;
    return us;
}

long ultrasonic_measure_inch(uint8_t trig_fun, uint8_t echo_fun, uint32_t timeout_us)
{
    long us = ultrasonic_measure_us(trig_fun, echo_fun, timeout_us);
    if(us>0)
        return us/74/2;
    return us;
}

