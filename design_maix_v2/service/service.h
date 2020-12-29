#ifndef __SERVICE_H__
#define __SERVICE_H__

#include "detect_service.h"
#include "distance_service.h"
#include "servo_service.h"

/*
 *  一条返回的消息固定格式：
 *  
 *  1、目标检测
 *  
 *  |--0-1-2-3-|--------4-------|---------------5-------------|-------6------|-------7------|-----8-----|--9~12---|------other-----|
 *  | 消息长度 | 0x66(固定魔数) | 消息返回类型(0x02/0x03/0x04) | 检测目标标志 | 检测车辆标志 | 检测人标志 | 测距值  | 二进制图片内容 |
 *  
 *  2、测距
 *  
 *  |--0-1-2-3--|--------4-------|---------------5-------------|--------6~8-------|---9~12--|-------other--------|
 *  |  消息长度 | 0x66(固定魔数) |      消息返回类型(0x05)      |    0x00(固定值)  | 测距值  | 消息返回类型(0x05) |
 *  
 *  3、控制舵机
 *  
 *  |--0-1-2-3-|--------4-------|---------------5-------------|-------6------|------7~12-----|-------other-------|
 *  | 消息长度 | 0x66(固定魔数) |      消息返回类型(0x06)      | 0x01(执行ok) | 0x00(固定值)  | 消息返回类型(0x06) |
 *  
*/

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
        ret = do_distance(arg, client_sock);
        break;
    case SERVOMOVE:
        ret = do_servo(arg, client_sock);
        break;
    default:
        break;
    }
    return ret;
}

#endif