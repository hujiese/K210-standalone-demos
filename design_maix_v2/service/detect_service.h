#ifndef __DETECTSERVICE_H__
#define __DETECTSERVICE_H__
/*
 *  消息格式： 
 * 
 *  |--0-1-2-3-|-------4-------|---------------5-------------|-------6------|-------7------|-----8-----|--9~12---|------other-----|
 *  | 消息长度 | 0x66(固定魔数) | 消息返回类型(0x20/0x21/0x22) | 检测目标标志 | 检测车辆标志 | 检测人标志 | 测距值  | 二进制图片内容 |
 * 
*/

/* 目标检测指令 'C' 'M' 'D' 0x01 0x20/0x21/0x22 0x0f
 *                               三个方向检测   
*/
#define DETECT 0x01
// 向前检测
#define FORWARD 0x20
// 向左检测
#define LEFT 0x21
// 向右检测
#define RIGHT 0x22

// 20类目标中车和人的索引号
#define BUSINDEX 5
#define CARINDEX 6
#define PERSONINDEX 14

// 用于LCD显示
extern int lcd_str_x_index(void);

#define MAX_IMG_LEN (1024 * 32)
#define NET_IMG_BUF (MAX_IMG_LEN + 4)

// 图像质量，60为最高
#define CONFIG_JPEG_COMPRESS_QUALITY 60
// jpeg区块长度
#define CONFIG_JPEG_BUF_LEN 64
// jpeg图片宽
#define CONFIG_CAMERA_RESOLUTION_WIDTH 320
// jpeg图片长
#define CONFIG_CAMERA_RESOLUTION_HEIGHT 240
// 转换jpeg图片缓冲区
static uint8_t jpeg_buf[CONFIG_JPEG_BUF_LEN * 2048];
static jpeg_encode_t jpeg_src, jpeg_out;

// 用于标识20类中有那些被检测到了
uint8_t class_detect_result[20] = {0x00};

/*
 *  返回给客户端的部分消息头
 *  detect_result[0]: 固定为0x66
 *  detect_result[1]: 检测类型，分为L/F/R三种，分别对应为0x03/0x02/0x04
 *  detect_result[2]: 是否检测到目标（人、车）
 *  detect_result[3]: 是否检测到车
 *  detect_result[4]: 是否检测到人
*/
uint8_t detect_result[5] = {0x66, 0x00, 0x00, 0x00, 0x00};
// // 用于填充返回消息类型
// #define RETFORWARD 0x02
// #define RETLEFT 0x03
// #define RETRIGHT 0x04

int convert_image2jpeg(uint8_t *image, int Quality)
{
    uint64_t v_tim;
    v_tim = sysctl_get_time_us();

    jpeg_src.w = CONFIG_CAMERA_RESOLUTION_WIDTH;
    jpeg_src.h = CONFIG_CAMERA_RESOLUTION_HEIGHT;
    jpeg_src.bpp = 2;
    jpeg_src.data = image;

    jpeg_out.w = jpeg_src.w;
    jpeg_out.h = jpeg_src.h;
    jpeg_out.bpp = CONFIG_JPEG_BUF_LEN * 1024;
    jpeg_out.data = jpeg_buf;

    v_tim = sysctl_get_time_us();
    uint8_t ret = jpeg_compress(&jpeg_src, &jpeg_out, Quality, 0);
    if (ret == 0)
    {
        printf("jpeg encode use %ld us\n", sysctl_get_time_us() - v_tim);
        printf("w:%d\th:%d\tbpp:%d\n", jpeg_out.w, jpeg_out.h, jpeg_out.bpp);
        printf("jpeg encode success!\n");
    }
    else
    {
        printf("jpeg encode failed\n");
    }

    return ret;
}

int send_msg_to_client(uint8_t *image_addr, uint8_t* detect_msg, int distance, uint8_t sock, int img_quality)
{
    int ret = 0;
    // 将rgb565图像转为jpeg数组
    if (convert_image2jpeg(image_addr, img_quality) == 0)
    {
        // 分配足够大的缓冲区保存转换的jpeg数组，发送数据格式为： 4字节图像长度+jpeg字节数组
        uint8_t* img_buf = (uint8_t *)malloc(NET_IMG_BUF);
        // 发送数据大小
        uint32_t i_len = 0;
        // 缓冲区前四个字节用于标记图像大小
        *(uint32_t*)(img_buf + i_len)= htonl(jpeg_out.bpp + 5 + 4);
        // 发送缓冲区指针偏移4字节（记录包长度）
        i_len += sizeof(uint32_t);
        // 将detect_msg数据写入缓冲区中,detect_msg: 魔数、检测方向、是否检测到目标、是否检测到车、是否检测到人
        memcpy(img_buf + i_len, detect_msg, 5);
        // 发送缓冲区指针偏移5字节（detect_msg长度）
        i_len += 5;
        *(uint32_t*)(img_buf + i_len)= htonl(distance);//记录前方障碍物距离
        // 发送缓冲区索引偏移4字节（障碍物距离）
        i_len += sizeof(uint32_t);
        // 将转换的jpeg数据写入缓冲区中
        memcpy(img_buf + i_len, jpeg_out.data, jpeg_out.bpp);
        // 发送长度增加发送图像长度
        i_len += jpeg_out.bpp;
        // 发送图像给客户端，这里的 SNDBUFSIZE 建议值为2048 (4096丢包太多，大于4096缓冲区会爆掉)
        uint32_t n_left;
        uint8_t* img_buf_w_buf = img_buf;
        n_left = i_len;
        if(send_data(sock, img_buf_w_buf, n_left) == -1)
        {
            printf("send data err !\n");
            lcd_draw_string(5, lcd_str_x_index(), "Network error !!!", RED);
            ret = -1;
        }
        free(img_buf);
    }

    return ret;
}

int do_detect(uint8_t arg, int client_sock)
{
    printk("cmd is detect.\n");
    if(msg[4] == FORWARD || msg[4] == LEFT || msg[4] == RIGHT)
    {
        memset(detect_result, 0, 5);
        detect_result[0] = 0x66; // 设置魔数
        int angle = 0;
        if(msg[4] == FORWARD)
        {
            angle = FORWARD_ANGLE;
            detect_result[1] = FORWARD;
        }
        else if(msg[4] == LEFT)
        {
            angle = LEFT_ANGLE;
            detect_result[1] = LEFT;
        }
        else if(msg[4] == RIGHT)
        {
            angle = RIGHT_ANGLE;
            detect_result[1] = RIGHT;
        }

        servo_move_angle(TIMER_PWM, TIMER_PWM_CHN0, SERVO_FREQ, angle);
        yolo_object_detect();
        if(class_detect_result[CARINDEX] == 0x01 || class_detect_result[BUSINDEX] == 0x01)
        {
            detect_result[2] = 0x01;
            detect_result[3] = 0x01;
            printk("detect car or bus !\n");
        }
        if(class_detect_result[PERSONINDEX] == 0x01)
        {
            detect_result[2] = 0x01;
            detect_result[4] = 0x01;
            printk("detect person !\n");
        }
        long distance = ultrasonic_measure_cm(FUNC_TRIG, FUNC_ECHO, 3000000);
        printk("%ld cm\n", distance);
        printk("detect result: %d,%d,%d\n", detect_result[2], detect_result[3], detect_result[4]);
        if(-1 == send_msg_to_client((uint8_t*)(g_ram_mux ? display_buf_addr1 : display_buf_addr2), detect_result, (int)distance, client_sock, CONFIG_JPEG_COMPRESS_QUALITY))
        {
            printf("send message fail !\n");
            printk("Client disconnect\n");
            client_stop(client_sock);
            return -1;
        }
        else
        {
            printf("send msg ok !\n");
        }
    }
    else
    {
        printk("detect unknow %d...\n", msg[4]);
    }
    return 0;
}

#endif