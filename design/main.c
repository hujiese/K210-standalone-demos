/**
* @par  Copyright (C): 2016-2022, SCUT
* @file         main.c
* @author       jackster
* @version      V1.0
* @date         2020.10.19
* @brief        摄像头显示当前画面，拍照存入SD卡中
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 摄像头采集当前画面，然后在LCD上显示;通过boot按键拍照将图片保存为jpeg格式存入sd卡根目录中。
*/
#include <stdio.h>
#include <string.h>
#include "dmac.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "ff.h"
#include "uarths.h"
#include "bsp.h"
#include "lcd.h"
#include "sysctl.h"
#include "uarths.h"
#include "ov2640.h"
#include "dvp_cam.h"
// #include "pin_config.h"
#include "plic.h"
#include "sleep.h"
#include "rgb2bmp.h"
#include "iomem.h"
#include "jpeg_encode.h"
#include "picojpeg.h"
#include "picojpeg_util.h"
#include "htonx.h"
#include "util.h"

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
// 统计拍照次数
volatile uint32_t g_count;
// 保存图片标志位
volatile uint8_t g_save_flag;
// WIFI热点和密码
char* WIFI_SSID = "DaGuLion";
char* WIFI_PASSWD = "abc360abc";
// 用于调试联网
#define TEST_NETWORK

#define MAX_IMG_LEN (1024 * 32)
#define NET_IMG_BUF (MAX_IMG_LEN + 4)

int main(void)
{
    // /* 设置拍照flag为0 */
    // g_save_flag = 0;
    
    // g_ram_mux = 0;

    hardware_init();    
    
    while (esp32_spi_connect_AP(WIFI_SSID, WIFI_PASSWD, 5) != 0);

    printf("wifi connection state: %d....\n", esp32_spi_is_connected());

#ifdef TEST_NETWORK
    test_network();
#endif

	uint16_t port = 8080;
	uint8_t sock = 1;
    allocate_socket(sock);
	bool started = esp32_spi_start_server(sock, 0, 0, port, TCP_MODE);
    if (!started)
	{
		printf("Server started on port %d\n", port);
	}
	else
	{
		printf("Server failed to start\n");
	}

    while(1)
    {
        int client_sock = esp32_spi_socket_available(sock);
        if (client_sock != 255 && client_sock != -1)
        {
            printk("New client connect %d\n", client_sock);
            allocate_socket(client_sock);

        }
        while(client_connected(&client_sock))
        {
            if(available(client_sock))
            {
                memset(msg, 0, MSGLEN);
                client_read(client_sock, msg, MSGLEN);
                printk("read from client:\n");
                if(msg[0] == 'C' && msg[1] == 'M' && msg[2] == 'D' && msg[5] == MSGENDFLAG)
                {
                    if(msg[3] == DETECT)
                    {
                        printk("cmd is detect.\n");
                        if(msg[4] == FORWARD)
                        {
                            printk("detect forward %d...\n", msg[4]);
                        }
                        else if(msg[4] == LEFT)
                        {
                            printk("detect left %d...\n", msg[4]);
                        }
                        else if(msg[4] == RIGHT)
                        {
                            printk("detect right %d...\n", msg[4]);
                        }
                        else
                        {
                            printk("detect unknow %d...\n", msg[4]);
                        }
                        
                    }
                }
                else
                {
                    printk("cmd error: %s", msg);
                }
            }
        }
        client_stop(client_sock);
    }

    return 0;
}


