#ifndef _FT6236U_H_
#define _FT6236U_H_

#include <stdint.h>

#define FT6236_I2C_ADDR         0x38

#define FT6236_IRQ_LEVEL        1

#define TP_PRES_DOWN            0x80  //触屏被按下	
#define TP_COORD_UD             0x40  //触摸坐标更新标记

/* FT6236 部分寄存器定义 */
#define FT_DEVIDE_MODE 			0x00   		//FT6236模式控制寄存器
#define FT_REG_NUM_FINGER       0x02		//触摸状态寄存器

#define FT_TP1_REG 				0X03	  	//第一个触摸点数据地址
#define FT_TP2_REG 				0X09		//第二个触摸点数据地址

#define FT_ID_G_MODE 			0xA4   		//FT6236中断模式控制寄存器
#define FT_ID_G_THGROUP			0x80   		//触摸有效值设置寄存器
#define FT_ID_G_PERIODACTIVE	0x88   		//激活状态周期设置寄存器  


typedef enum _rst_level{
    LEVEL_LOW = 0,
    LEVEL_HIGH = 1
} rst_level_t;

/* ft6236触摸结构体 */
typedef struct
{
    // 触摸状态 b7:按下1/松开0; b6:0没有按键按下/1有按键按下;
    // bit5-bit1:保留；bit0触摸点按下有效标志，有效为1
    uint8_t touch_state;
    uint16_t touch_x;
    uint16_t touch_y;
} ft6236_touch_point_t;

extern ft6236_touch_point_t ft6236;


void ft6236_init(void);
void ft6236_scan(void);
void ft6236_reset_pin(rst_level_t level);
void ft6236_hardware_init(void);


#endif /* _FT6236U_H_ */
