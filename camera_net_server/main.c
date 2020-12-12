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
#include "unistd.h"
#include "fpioa.h"
#include "sdcard.h"
#include "gpiohs.h"
#include "ff.h"
#include "i2s.h"
#include "uarths.h"
#include "bsp.h"
#include "lcd.h"
#include "st7789.h"
#include "sysctl.h"
#include "uarths.h"
#include "ov2640.h"
#include "dvp_cam.h"
#include "pin_config.h"
#include "plic.h"
#include "sleep.h"
#include "rgb2bmp.h"
#include "iomem.h"
#include "jpeg_encode.h"
#include "picojpeg.h"
#include "picojpeg_util.h"
#include "esp32_spi_io.h"
#include "esp32_spi.h"
#include "htonx.h"
#include "client_utils.h"
// 图像质量，60为最高
#define CONFIG_JPEG_COMPRESS_QUALITY 60
// jpeg区块长度
#define CONFIG_JPEG_BUF_LEN 64
// jpeg图片宽
#define CONFIG_CAMERA_RESOLUTION_WIDTH 320
// jpeg图片长
#define CONFIG_CAMERA_RESOLUTION_HEIGHT 240
// 一次性发送数据块大小，建议长度为2048
#define SNDBUFSIZE 2048
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
// 服务器IP和端口
uint8_t ip[] = {192, 168, 137, 1};
uint16_t port = 18899;
// 用于调试联网
#define TEST_NETWORK

#define MAX_IMG_LEN (1024 * 32)
#define NET_IMG_BUF (MAX_IMG_LEN + 4)


uint8_t msg[8];


/**
* Function       io_set_power
* @author        jackster
* @date          2020.10.19
* @brief         设置摄像头和显示器电源域电压值
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void io_set_power(void)
{
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

/**
* Function       hardware_init
* @author        jackster
* @date          2020.10.19
* @brief         硬件初始化，绑定GPIO口
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void hardware_init(void)
{
    /* lcd */
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);

    /* DVP camera */
    fpioa_set_function(PIN_DVP_RST,   FUNC_CMOS_RST);
    fpioa_set_function(PIN_DVP_PWDN,  FUNC_CMOS_PWDN);
    fpioa_set_function(PIN_DVP_XCLK,  FUNC_CMOS_XCLK);
    fpioa_set_function(PIN_DVP_VSYNC, FUNC_CMOS_VSYNC);
    fpioa_set_function(PIN_DVP_HSYNC, FUNC_CMOS_HREF);
    fpioa_set_function(PIN_DVP_PCLK,  FUNC_CMOS_PCLK);
    fpioa_set_function(PIN_DVP_SCL,   FUNC_SCCB_SCLK);
    fpioa_set_function(PIN_DVP_SDA,   FUNC_SCCB_SDA);

    /* KEY */
    fpioa_set_function(PIN_KEY, FUNC_KEY);

    /* ESP32 */
    fpioa_set_function(PIN_ESP_CS,   FUNC_ESP_SPI_CS);
    fpioa_set_function(PIN_ESP_RST,  FUNC_ESP_SPI_RST);
    fpioa_set_function(PIN_ESP_RDY,  FUNC_ESP_SPI_RDY);
    fpioa_set_function(PIN_ESP_MOSI, FUNC_ESP_SPI_MOSI);
    fpioa_set_function(PIN_ESP_MISO, FUNC_ESP_SPI_MISO);
    fpioa_set_function(PIN_ESP_CLK,  FUNC_ESP_SPI_CLK);

    /* 使能SPI0和DVP */
    sysctl_set_spi0_dvp_data(1);
}

/**
* Function       key_irq_cb
* @author        jackster
* @date          2020.10.19
* @brief         按键key中断回调函数
* @param[in]     ctx 回调参数
* @param[out]    void
* @retval        0
* @par History   无
*/
int key_irq_cb(void* ctx)
{
    g_save_flag = 1;
    printf("key down ...\n");
    return 0;
}

/**
* Function       init_key
* @author        jackster
* @date          2020.10.19
* @brief         初始化按键key
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void init_key(void)
{
    /* 设置按键的GPIO模式为上拉输入 */
    gpiohs_set_drive_mode(KEY_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    /* 设置按键的GPIO电平触发模式为下降沿触发 */
    gpiohs_set_pin_edge(KEY_GPIONUM, GPIO_PE_FALLING);
    /* 设置按键GPIO口的中断回调 */
    gpiohs_irq_register(KEY_GPIONUM, 2, key_irq_cb, &g_count);
}

/**
* Function       convert_image2jpeg
* @author        jackster
* @date          2020.10.19
* @brief         将 rgb565图像字节数组转为jpeg图片数组
* @param[in]     image rgb565图像字节数组;Quality 转换质量
* @param[out]    ret 值
* @retval        void
* @par History   无
*/
static int convert_image2jpeg(uint8_t *image, int Quality)
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

/**
* Function       connect_server_by_ip_port
* @author        jackster
* @date          2020.10.26
* @brief         连接ip:port服务器，并返回一个连接套接字
* @param[in]     host 点分十进制字节Ip数组; port 16位长度端口
* @param[out]    无
* @retval        失败返回0xff;成功返回一个连接套接字
* @par History   无
*/
uint8_t connect_server_by_ip_port(uint8_t* host, uint16_t port)
{
    uint8_t sock = esp32_spi_get_socket();

    if (sock != 0xff)
    {
        if (esp32_spi_socket_connect(sock, ip, 0, port, TCP_MODE) < 0)
        {
            return 0xff;
        }
    }
    else
    {
        return 0xff;
    }
    return sock;
}

void reconnect_sock(uint8_t* host, uint16_t port, uint8_t sock) 
{
    esp32_spi_socket_close(sock);
    sock = connect_server_by_ip_port(ip, port);
    if(sock == 0xff)
    {
        printf("connect server error !\n");
        return -1;
    }
}

int send_to_server(uint8_t sock, uint8_t* img_buf_w_buf, uint32_t n_left)
{
    uint32_t n_written;
    while(n_left > 0)
    {
        if((n_written = esp32_spi_socket_write(sock, img_buf_w_buf, SNDBUFSIZE)) <= 0)
        {
            printf("wirte socket err1 !\n");
            return -1;
        }
        n_left -= n_written;
        img_buf_w_buf += n_written;
        if(n_left < SNDBUFSIZE)
        {
            if((n_written = esp32_spi_socket_write(sock, img_buf_w_buf, n_left)) <= 0)
            {
                printf("wirte socket err2 !\n");
                return -1;
            }
            break;
        }
    }
    return 0;
}

/**
* Function       send_jpeg_server
* @author        jackster
* @date          2020.10.26
* @brief         将 rgb565图像字节数组转换为jpeg格式图片上传到ip:port服务器上
* @param[in]     image_addr 摄像头拍摄rgb565图片地址;sock 连接套接字; ip:port 服务器ip和端口;img_quality 图像质量
* @param[out]    成功返回1, 失败返回0
* @retval        void
* @par History   无
*/
int send_jpeg_server(uint8_t *image_addr, uint8_t sock, uint8_t* ip, uint16_t port, int img_quality)
{
    // 将rgb565图像转为jpeg数组
    if (convert_image2jpeg(image_addr, img_quality) == 0)
    {
        // 分配足够大的缓冲区保存转换的jpeg数组，发送数据格式为： 4字节图像长度+jpeg字节数组
        uint8_t* img_buf = (uint8_t *)malloc(NET_IMG_BUF);
        // 魔数、检测方向、是否检测到目标、是否检测到车、是否检测到人
        uint8_t partMsg[5] = {0x66, 0x02, 0x01, 0x01, 0x00};
        // 发送数据大小
        uint32_t i_len = 0;
        // 缓冲区前四个字节用于标记图像大小
        *(uint32_t*)(img_buf + i_len)= htonl(jpeg_out.bpp + 5 + 4);
        // 发送缓冲区指针偏移4字节（记录包长度）
        i_len += sizeof(uint32_t);
        // 将partMsg数据写入缓冲区中
        memcpy(img_buf + i_len, partMsg, 5);
        // 发送缓冲区指针偏移5字节（partMsg长度）
        i_len += 5;
        *(uint32_t*)(img_buf + i_len)= htonl(154);//记录前方障碍物距离
        // 发送缓冲区索引偏移4字节（障碍物距离）
        i_len += sizeof(uint32_t);
        // 将转换的jpeg数据写入缓冲区中
        memcpy(img_buf + i_len, jpeg_out.data, jpeg_out.bpp);
        // 发送长度增加发送图像长度
        i_len += jpeg_out.bpp;
        // 发送图像给远程服务器，这里的 SNDBUFSIZE 建议值为2048 (4096丢包太多，大于4096缓冲区会爆掉)
        uint32_t n_left;
        uint8_t* img_buf_w_buf = img_buf;
        n_left = i_len;
        while(send_to_server(sock, img_buf_w_buf, n_left) == -1)
        {
            printf("send data err, reconnect and resend2!\n");
            reconnect_sock(ip, port, sock);
        }
        printf("send jpeg image ok...\n");
        free(img_buf);
        return 1;
    }

    return 0;
}

/**
* Function       main
* @author        jackster
* @date          2020.10.19
* @brief         主函数，程序的入口
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
int main(void)
{
    /* 硬件引脚初始化 */
    hardware_init();
    
    /* 设置IO口电压 */
    io_set_power();

    /* 设置系统时钟和DVP时钟 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 300000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

    /* 系统中断初始化，使能全局中断*/
    plic_init();
    sysctl_enable_irq();
    
    /* 初始化LCD */
    lcd_init();

    // 初始化按键key
    init_key();

    /* 初始化摄像头 */
    ov2640_init();

    /* 设置拍照flag为0 */
    g_save_flag = 0;
    
    g_ram_mux = 0;

    esp32_spi_config_io(FUNC_ESP_SPI_CS - FUNC_GPIOHS0, FUNC_ESP_SPI_RST, FUNC_ESP_SPI_RDY - FUNC_GPIOHS0, FUNC_ESP_SPI_MOSI - FUNC_GPIOHS0, FUNC_ESP_SPI_MISO - FUNC_GPIOHS0, FUNC_ESP_SPI_CLK - FUNC_GPIOHS0);

    esp32_spi_init();
    
    while (esp32_spi_connect_AP(WIFI_SSID, WIFI_PASSWD, 5) != 0);

    printf("wifi connection state: %d....\n", esp32_spi_is_connected());

#ifdef TEST_NETWORK
    printf("------------------------------------------\n");

    int ping_www_count = 5;
    int ping_www_success = 0;
    while(ping_www_count--)
    {
        ping_www_success = esp32_spi_ping("www.baidu.com", 1, 255);
        printf("ping www.baidu.com: %d ms\n", ping_www_success);
        if(ping_www_success > 0)
            break;
        sleep(1);
    }

    printf("------------------------------------------\n");

    int ping_ip_count = 5;
    int ping_ip_success = 0;
    while(ping_ip_count--)
    {
        ping_ip_success = esp32_spi_ping(ip, 0, 255);
        printf("ping 192.168.137.1: %d ms\n", ping_ip_success);
        if(ping_ip_success >= 0 && ping_ip_success < 255)
            break;
        sleep(1);
    }

    printf("------------------------------------------\n");

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
                memset(msg, 0, 8);
                client_read(client_sock, msg, 8);
                printk("read from client: %s\n", msg);
            }
        }
        client_stop(client_sock);
    }

    return 0;
}


