#ifndef __SERVOSERVICE_H__
#define __SERVOSERVICE_H__

 /*
 *  舵机服务消息格式：
 *  
 *  |--0-1-2-3-|--------4-------|---------------5-------------|-------6------|------7~12-----|-------other-------|
 *  | 消息长度 | 0x66(固定魔数) |      消息返回类型(0x06)      | 0x01(执行ok) | 0x00(固定值)  | 消息返回类型(0x06) |
 * 
*/ 

/* 舵机移动指令 'C' 'M' 'D' 0x03 0xxx 0x0f
 *                               0xxx为转动角度(0~180，需要转换为-90~90)
*/
#define SERVOMOVE 0x06
// 舵机服务消息固定长度为10个字节
#define SERVO_PAYLOAD_LEN 10
// 舵机消息总长度
#define SERVO_MSG_LEN 14
// 舵机服务返回给客户端的部分消息头
uint8_t servo_result[SERVO_PAYLOAD_LEN] = {0x66, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06};

int do_servo(uint8_t arg, int client_sock)
{
    printk("cmd is servo.\n");
    int angle = (int)arg - 90;// arg为0~180度，需要转换为-90~90度
    // 测距
    servo_move_angle(TIMER_PWM, TIMER_PWM_CHN0, SERVO_FREQ, angle);
    // 创建发送缓冲区
    uint8_t* servo_buf = (uint8_t *)malloc(SERVO_MSG_LEN);
    // 发送数据大小
    uint32_t i_len = 0;
    // 缓冲区前四个字节用于标记有效荷载大小
    *(uint32_t*)(servo_buf + i_len)= htonl(SERVO_PAYLOAD_LEN);
    // 发送缓冲区指针偏移4字节（记录包长度）
    i_len += sizeof(uint32_t);
    memcpy(servo_buf + i_len, servo_result, 10);
    // 发送缓冲区指针偏移5字节（detect_msg长度）
    i_len += 10;
    uint32_t n_left = i_len;
    uint32_t n_written;
    while(n_left > 0)
    {
        if((n_written = esp32_spi_socket_write(client_sock, servo_buf, SERVO_MSG_LEN)) <= 0)
        {
            printf("wirte socket err !\n");
            return -1;
        }
        n_left -= n_written;
    }
    free(servo_buf);
    return 0;
}
#endif