#ifndef __DISTANCESERVICE_H__
#define __DISTANCESERVICE_H__

/*
 *  测距服务消息格式：
 *  
 *  |--0-1-2-3--|--------4-------|---------------5-------------|--------6~8-------|---9~12--|-------other--------|
 *  |  消息长度 | 0x66(固定魔数) |      消息返回类型(0x05)      |    0x00(固定值)  | 测距值  | 消息返回类型(0x05) |
*/ 

/* 测距指令 'C' 'M' 'D' 0x02 0x00 0x0f
 *                           参数0x00保留    
*/
#define DISTANCE 0x05
// 测距回复消息固定长度为10个字节
#define DISTANCE_PAYLOAD_LEN 10
// 测距消息总长度
#define DISTANCE_MSG_LEN 14
// 测距返回给客户端的部分消息头
uint8_t distance_result[5] = {0x66, 0x05, 0x00, 0x00, 0x00};

int do_distance(uint8_t arg, int client_sock)
{
    printk("cmd is distance.\n");
    int ret = 0;
    // 测距
    long distance = ultrasonic_measure_cm(FUNC_TRIG, FUNC_ECHO, 3000000);
    // 创建发送缓冲区
    uint8_t* distance_buf = (uint8_t *)malloc(DISTANCE_MSG_LEN);
    // 发送数据大小
    uint32_t i_len = 0;
    // 缓冲区前四个字节用于标记有效荷载大小
    *(uint32_t*)(distance_buf + i_len)= htonl(DISTANCE_PAYLOAD_LEN);
    // 发送缓冲区指针偏移4字节（记录包长度）
    i_len += sizeof(uint32_t);
    memcpy(distance_buf + i_len, distance_result, 5);
    // 发送缓冲区指针偏移5字节（detect_msg长度）
    i_len += 5;
    *(uint32_t*)(distance_buf + i_len)= htonl(distance);//记录前方障碍物距离
    // 发送缓冲区索引偏移4字节（障碍物距离）
    i_len += sizeof(uint32_t);
    *(uint32_t*)(distance_buf + i_len)= DISTANCE; //消息结尾
    i_len += 1;
    uint32_t n_left = i_len;
    uint32_t n_written;
    while(n_left > 0)
    {
        if((n_written = esp32_spi_socket_write(client_sock, distance_buf, n_left)) <= 0)
        {
            printf("wirte socket err !\n");
            ret = -1;
            break;
        }
        n_left -= n_written;
    }
    free(distance_buf);
    return ret;
}
#endif