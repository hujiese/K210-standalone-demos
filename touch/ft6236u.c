#include <stdio.h>

#include "ft6236u.h"
#include "i2c_ctl.h"
#include "pin_config.h"

#include "gpiohs.h"
#include "sleep.h"
#include "plic.h"
#include "sysctl.h"

/* ft6236结构体变量 */
ft6236_touch_point_t ft6236;

/* I2C写数据 */
static void ft_i2c_write(uint8_t reg, uint8_t data)
{
    i2c_hd_write(FT6236_I2C_ADDR, reg, data);
}

/* I2C读数据 */
static void ft_i2c_read(uint8_t reg, uint8_t *data_buf, uint16_t length)
{
    i2c_hd_read(FT6236_I2C_ADDR, reg, data_buf, length);
}

/* 初始化ft6236 */
void ft6236_init(void)
{
    ft6236.touch_state = 0;
    ft6236.touch_x = 0;
    ft6236.touch_y = 0;

    /* 硬件初始化 */
    ft6236_hardware_init();

    /* 软件初始化 */
    i2c_hardware_init(FT6236_I2C_ADDR);
    ft_i2c_write(FT_DEVIDE_MODE, 0x00);
    /* 设置触摸有效值，越小越灵敏，def=0xbb */
    ft_i2c_write(FT_ID_G_THGROUP, 0x12);    // 0x22
    /* 工作扫描周期，用于控制报点率，def=0x08, 0x04~0x14 */
    ft_i2c_write(FT_ID_G_PERIODACTIVE, 0x06);
}

/* 重置FT_RST引脚GPIO电平 */
void ft6236_reset_pin(rst_level_t level)
{
    gpiohs_set_pin(FT_RST_GPIONUM, level);
}

/* 中断回调函数，修改touch_state的状态为有触摸 */
void ft6236_isr_cb(void)
{
    ft6236.touch_state |= TP_COORD_UD;
}

/* FT6236硬件引脚初始化 */
void ft6236_hardware_init(void)
{
    /* 与屏幕使用不同复位引脚时设置为1 */
    #if (0) 
    {
        gpiohs_set_drive_mode(FT_RST_GOIONUM, GPIO_DM_OUTPUT);
        ft6236_reset_pin(LEVEL_LOW);
        msleep(50);
        ft6236_reset_pin(LEVEL_HIGH);
        msleep(120);
    }
    #endif

    gpiohs_set_drive_mode(FT_INT_GPIONUM, GPIO_DM_INPUT);
    gpiohs_set_pin_edge(FT_INT_GPIONUM, GPIO_PE_RISING);
    gpiohs_irq_register(FT_INT_GPIONUM, FT6236_IRQ_LEVEL, ft6236_isr_cb, NULL);
    msleep(5);
    
}

/* 扫描FT6236，并读取坐标的XY值 */
void ft6236_scan(void)
{
    uint8_t sta = 0;
    uint8_t data[4] = {0};
    ft_i2c_read(FT_REG_NUM_FINGER, &sta, 1);
    // printf("read-point:%x \n", sta);
    if (sta & 0x0f)
    {
        ft6236.touch_state = ~(0xFF << (sta & 0x0F));
        if (ft6236.touch_state & (1 << 0))
        {
            ft_i2c_read(FT_TP1_REG, data, 4);

            uint16_t temp_y = ((uint16_t)(data[0] & 0x0f) << 8) + data[1];
            ft6236.touch_y = (uint16_t)(240 - temp_y);
            // uint16_t temp_x = ((uint16_t)(data[2] & 0x0f) << 8) + data[3];
            ft6236.touch_x = ((uint16_t)(data[2] & 0x0f) << 8) + data[3];
            
            if ((data[0] & 0xC0) != 0x80)
            {
                ft6236.touch_x = ft6236.touch_y = 0;
                return;
            }
        }
        /* 标记触摸按下 */
        ft6236.touch_state |= TP_PRES_DOWN;
    }
    else
    {
        /* 之前被标记过的 */
        if (ft6236.touch_state & TP_PRES_DOWN)
        {
            /* 触摸松开标记 */
            ft6236.touch_state &= ~0x80;
        }
        else
        {
            /* 清空坐标值和清除触摸有效标记 */
            ft6236.touch_x = 0;
            ft6236.touch_y = 0;
            ft6236.touch_state &= 0xe0;
        }
    }
}
