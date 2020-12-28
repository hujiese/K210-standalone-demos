#ifndef __SERVICE_H__
#define __SERVICE_H__

#include "detect_service.h"

/* 测距指令 'C' 'M' 'D' 0x02 0x00 0x0f
 *                           参数0x00保留    
*/
#define DISTANCE 0x02

/* 舵机移动指令 'C' 'M' 'D' 0x03 0xxx 0x0f
 *                               0xxx为转动角度(0~180，需要转换为-90~90)
*/
#define SERVOMOVE 0x03

/* 命令结束标志 */
#define MSGENDFLAG 0x0f

int do_cmd(uint8_t cmd, uint8_t arg, int client_sock)
{
    int ret = 0;
    switch (cmd)
    {
    case DETECT:
        ret = do_detect(arg, client_sock);
        break;
    case DISTANCE:
        break;
    case SERVOMOVE:
        break;
    default:
        break;
    }
    return ret;
}

#endif