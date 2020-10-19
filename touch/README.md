# This is ft6236 register data

| 寄存器页面 | 工作模式           |           |                          |                              |          |            |          |                                                              |
| ---------- | ------------------ | :-------: | ------------------------ | ---------------------------- | -------- | ---------- | -------- | :----------------------------------------------------------: |
| 切换方式   | 写0x00到寄存器0x00 |           |                          |                              |          |            |          |                                                              |
|            |                    |           |                          |                              |          |            |          |                                                              |
| 寄存器地址 | 长度               |   读写    | 寄存器符号               | 寄存器名称                   | 取值范围 | 寄存器描述 |          |                                                              |
| 默认值     | 最小值             |  最大值   |                          |                              |          |            |          |                                                              |
| 0x00       | 1 Byte             | 读写(RW)  | Mode_Switch              | 寄存器页面（模式）切换       |          |            |          |          写0x40到寄存器0x00切换到工厂模式读出值为0           |
| 0x01       | 1 Byte             | 只读(RO)  | Reserved                 |                              | 0x00     |            |          |                                                              |
| 0x02       | 1 Byte             | 只读(RO)  | TD_STATUS                | 报点个数                     | 0x00     | 0x00       | 0x02     |                      最大支持同时报两点                      |
| 0x03       | 1 Byte             | 只读(RO)  | P1_XH                    | 第1点的X坐标高4位            | 0xFF     |            |          | bit7-bit6为第1点触摸事件标志，bit3-bit0为第1点的X坐标高4位。 |
| 0x04       | 1 Byte             | 只读(RO)  | P1_XL                    | 第1点的X坐标低8位            | 0xFF     |            |          |                                                              |
| 0x05       | 1 Byte             | 只读(RO)  | P1_YH                    | 第1点的Y坐标高4位            | 0xFF     |            |          |     bit7-bit4为第1点的ID，bit3-bit0为第1点的Y坐标高4位。     |
| 0x06       | 1 Byte             | 只读(RO)  | P1_YL                    | 第1点的Y坐标低8位            | 0xFF     |            |          |                                                              |
| 0x07       | 1 Byte             | 只读(RO)  | P1_WEIGHT                |                              | 0xFF     |            |          |                                                              |
| 0x08       | 1 Byte             | 只读(RO)  | P1_MISC                  |                              | 0xFF     |            |          |                                                              |
| 0x09       | 1 Byte             | 只读(RO)  | P2_XH                    | 第2点的X坐标高4位            | 0xFF     |            |          | bit7-bit6为第2点触摸事件标志，bit3-bit0为第1点的X坐标高4位。 |
| 0x0A       | 1 Byte             | 只读(RO)  | P2_XL                    | 第2点的X坐标低8位            | 0xFF     |            |          |                                                              |
| 0x0B       | 1 Byte             | 只读(RO)  | P2_YH                    | 第2点的Y坐标高4位            | 0xFF     |            |          |     bit7-bit4为第2点的ID，bit3-bit0为第2点的Y坐标高4位。     |
| 0x0C       | 1 Byte             | 只读(RO)  | P2_YL                    | 第2点的Y坐标低8位            | 0xFF     |            |          |                                                              |
| 0x0D       | 1 Byte             | 只读(RO)  | P2_WEIGHT                |                              | 0xFF     |            |          |                                                              |
| 0x0E       | 1 Byte             | 只读(RO)  | P2_MISC                  |                              | 0xFF     |            |          |                                                              |
| 0x0F～0x7F |                    |           | Reserved                 |                              |          |            |          |                                                              |
| 0x80       | 1 Byte             | 读写(RW)  | ID_G_THGROUP             | 触摸阈值                     | 0xBB     |            |          |                        触摸阈值 / 16                         |
| 0x81～0x84 |                    |           | Reserved                 |                              |          |            |          |                                                              |
| 0x85       | 1 Byte             | 读写(RW)  | ID_G_THDIFF              | 点滤波范围阈值               | 0xA0     | 0x00       | 0xFF     |                     约等于RV_G_THDIFF/16                     |
| 0x86       | 1 Byte             | 读写(RW)  | ID_G_CTRL                | monitor模式开关              | 0x01     | 0x00       | 0x01     |       是否允许进入monitor模式 0x01 : 允许 0x00 : 禁止        |
| 0x87       | 1 Byte             | 读写(RW)  | ID_G_TIMEENTERMONITOR    | 没触摸进入monitor延时        | 0x1E     | 0x00       | 0x64     | 在指定时间内没有触摸则进入MONITOR状态，需要与参数“monitor模式开关”配合使用。单位为秒。 |
| 0x88       | 1 Byte             | 读写(RW)  | ID_G_PERIODACTIVE        | 工作模式扫描周期             | 0x08     | 0x04       | 0x14     |                        用于控制报点率                        |
| 0x89       | 1 Byte             | 读写(RW)  | ID_G_PERIODMONITOR       | MONITOR模式扫描周期          | 0x08     | 0x04       | 0x14     |                        用于控制报点率                        |
| 0x8A       |                    | 读写(RW)  | Reserved                 |                              |          |            |          |                                                              |
| 0x8B       | 1 Byte             | 读写(RW)  | ID_G_FREQ_HOPPING_EN     | 充电器标识                   | 0x00     | 0x00       | 0x01     | 需要Host在插拔充电器时操作该寄存器 0x00:充电器plug out 0x01:充电器plug in |
| 0x8C～0x95 |                    |           | Reserved                 |                              |          |            |          |                                                              |
| 0x96       | 1 Byte             | 读写(RW)  | ID_G_TEST_MODE_FILTER    | 量产测试模式下Alpha滤波使能  | 0x00     | 0x00       | 0x01     |                                                              |
| 0x97～0x9E |                    |           | Reserved                 |                              |          |            |          |                                                              |
| 0x9F       | 1 Byte             | 只读(RO)  | ID_G_CIPHER_MID          | 芯片代号（中字节）           | 0x26     | 0x00       | 0xFF     |                                                              |
| 0xA0       | 1 Byte             | 只读(RO)  | ID_G_CIPHER_LOW          | 芯片代号（低字节）           | 0x00     | 0x00       | 0x03     | 0x00: Ft6236G                                      0x01: Ft6336G                                      0x02: Ft6336U                                      0x03: Ft6426 |
| 0xA1       | 1 Byte             | 只读(RO)  | ID_G_LIB_VERSION_H       | APP 库文件版本号高字节       | 0x10     | 0x00       | 0xFF     |                    APP 库文件版本号高字节                    |
| 0xA2       | 1 Byte             | 只读(RO)  | ID_G_LIB_VERSION_L       | APP 库文件版本号低字节       | 0x01     | 0x00       | 0xFF     |                    APP 库文件版本号低字节                    |
| 0xA3       | 1 Byte             | 只读(RO)  | ID_G_CIPHER_HIGH         | 芯片代号（高字节）           | 0x64     | 0x00       | 0xFF     |                                                              |
| 0xA4       | 1 Byte             | 读写(RW)  | ID_G_MODE                | 报点INT模式                  | 0x01     | 0x00       | 0x01     | 用于控制报点INT模式 0x00 : 不拉长报点时INT低电平时间 0x00 : 拉长报点时INT低电平时间 |
| 0xA5       | 1 Byte             | 读写(RW)  | ID_G_PMODE               | 芯片工作模式                 | 0x00     | 0x00       | 0x03     | 功耗模式 0x00 : P_ACTIVE 0x01 : P_MONITOR 0x02 : P_STANDBY 0x03 : P_HIBERNATE |
| 0xA6       | 1 Byte             | 只读(RO)  | ID_G_FIRMID              | Firmware Verson              | 0x00     | 0x00       | 0xFF     |                                                              |
| 0xA7       |                    |           | Reserved                 |                              |          |            |          |                                                              |
| 0xA8       | 1 Byte             | 只读(RO)  | ID_G_FOCALTECH_ID        | VENDOR ID                    | 0x11     | 0x00       | 0xFF     |                                                              |
| 0xA9       | 1 Byte             | 读写(RW)  | ID_G_VIRTUAL_KEY_THRES   | 量产测试模式下的虚拟按键阈值 | 0X19     | 0x00       | 0xFF     |               约等于VIRTUAL_KEY_THRESHOLD / 40               |
| 0xAA～0xAC |                    |           | Reserved                 |                              |          |            |          |                                                              |
| 0xAD       | 1 Byte             | 读写(RW)  | ID_G_IS_CALLING          | 通话标志字                   | 0x00     | 0x00       | 0x01     |                 用于HOST通知芯片是否在通话中                 |
| 0xAE       | 1 Byte             | 读写(RW)  | ID_G_FACTORY_MODE        | 工厂模式种类                 | 0x00     | 0x00       | 0x02     |   0x00 : F_NORMAL 0x01 : F_TESTMODE_1 0x02 : F_TESTMODE_2    |
| 0xAF       | 1 Byte             | 只读(RO)  | ID_G_RELEASE_CODE_ID     | 公版release code ID          | 0x01     | 0x00       | 0x01     |                                                              |
| 0xB0       | 1 Byte             | 读写(RW)  | ID_G_FACE_DEC_MODE       | 近距离感应使能               | 0x00     | 0x00       | 0x01     |                 0x00 : Disable 0x01 : Enable                 |
| 0xB1-0xBB  |                    | 读写(RW)  | Reserved                 |                              |          |            |          |                                                              |
| 0xBC       | 1 Byte             |  写（W)   | ID_G_STATE               | 工作模式                     | 0x01     | 0x00       | 0x04     | 写0xAA，0x55到这个寄存器触发升级流程 0x00: InFO模式 0x01: 正常工作模式 0x03: 工厂模式 0x04: 自动校正模式 |
| 0xBD～0xCF |                    | 读写(RW)  | Reserved                 |                              |          |            |          |                                                              |
|            |                    |           |                          |                              |          |            |          |                                                              |
|            |                    |           |                          |                              |          |            |          |                                                              |
|            |                    |           |                          |                              |          |            |          |                                                              |
| 0xD0       | 1 Byte             | 读写(RW)  | ID_G_SPEC_GESTURE_ENABLE | 特殊手势模式使能             | 0x00     | 0x00       | 0x01     | A. 进入手势状态,正常工作状态下向0xd0寄存器写1,退出手势0xd0寄存器写0； B. 使能手势,0xd1,0xd2,0xd5—0xd8寄存器.对应的bit写1,使能相应的手势； |
| 0xD1～0xDA |                    | 读写（RW) | Reserved                 |                              |          |            |          |                                                              |
|            | Bit7               |   Bit6    | Bit5                     | Bit4                         | Bit3     | Bit2       | Bit1     |                             Bit0                             |
| 0xD1       | 保留               |   保留    | 数字，字符识别总开关     | 双击使能                     | 下滑直线 | 上滑直线   | 右滑直线 |                           左滑直线                           |
| 0xD2       | d                  |     a     | g                        | c                            | e        | m          | w        |                              o                               |
| 0xD3       | 手势ID号           |           |                          |                              |          |            |          |                                                              |
| 0xD4       | 保留               |           |                          |                              |          |            |          |                                                              |
| 0xD5       | u                  |     s     | P                        | L                            | q        | b          | 保留     |                              n                               |
| 0xD6       | 保留               |   保留    | △                        | v                            | ^        | >          | 保留     |                              @                               |
| 0xD7       | 保留               |   保留    | 2                        | 8                            | 7        | 9          | 6        |                              3                               |
| 0xD8       | 保留               |   保留    | 保留                     | 保留                         | r        | y          | k        |                              h                               |
| 0xD9       | 保留               |           |                          |                              |          |            |          |                                                              |
| 0xDA       | DEBUG USE          |           |                          |                              |          |            |          |                                                              |