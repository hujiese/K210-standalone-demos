#include <stdio.h>
#include <string.h>
#include "dmac.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "ff.h"
#include "uarths.h"
#include "bsp.h"
#include "sysctl.h"
#include "uarths.h"
#include "plic.h"
#include "sleep.h"
#include "iomem.h"
#include "util.h"
#include "wdt.h"

// WIFI热点和密码
char* WIFI_SSID = "DaGuLion";
char* WIFI_PASSWD = "abc360abc";
// 用于调试联网
#define TEST_NETWORK
// #define TEST_CAMERA_DETECT
uint16_t port = 8080;
uint8_t sock = 1;

#define WDT_TIMEOUT_REBOOT    1

int wdt0_irq_cb(void *ctx)
{
    #if WDT_TIMEOUT_REBOOT
    printf("%s:The system will reboot soon!\n", __func__);
    while(1);
    #else
    printf("%s:The system is busy but not reboot!\n", __func__);
    wdt_clear_interrupt(WDT_DEVICE_0);
    #endif
    return 0;
}

int main(void)
{
    hardware_init();    
    lcd_draw_string(5, lcd_str_x_index(), "Hardware init ok !", BLACK);  
    lcd_draw_string(5, lcd_str_x_index(), "Connecting WIFI...", BLACK);  
    while (esp32_spi_connect_AP(WIFI_SSID, WIFI_PASSWD, 5) != 0);

    if(esp32_spi_is_connected() == 0)
    {
        lcd_draw_string(5, lcd_str_x_index(), "WIFI Connect ok !", BLACK);
    }
    else
    {
        lcd_draw_string(5, lcd_str_x_index(), "WIFI Connect fail !", RED);
    }

#ifdef TEST_NETWORK
    lcd_draw_string(5, lcd_str_x_index(), "Testing network...", BLACK);
    if(test_network() > 0)
        lcd_draw_string(5, lcd_str_x_index(), "Network ok !", BLACK);
    else
        lcd_draw_string(5, lcd_str_x_index(), "Network is bad, please reset !", RED);
#endif

    g_ram_mux = 0;
	bool started = esp32_spi_start_server(sock, 0, 0, port, TCP_MODE);
    if (!started)
	{
        lcd_draw_string(5, lcd_str_x_index(), "Server started on port 8080.", BLACK);
	}
	else
	{
        lcd_draw_string(5, lcd_str_x_index(), "Server start fail !", RED);
	}
    /* 启动看门狗，设置超时时间为10秒后调用中断函数wdt0_irq_cb */
    wdt_init(WDT_DEVICE_0, 10000, wdt0_irq_cb, NULL);
    while(1)
    {
        int client_sock = esp32_spi_socket_available(sock);
        if (client_sock != 255 && client_sock != -1)
        {
            lcd_draw_string(5, lcd_str_x_index(), "New client connect !", BLUE);
            lcd_clear(WHITE);
        }
        while(client_connected(&client_sock))
        {
#ifdef TEST_CAMERA_DETECT
            yolo_object_detect();
#endif

            if(available(client_sock))
            {
                memset(msg, 0, MSGLEN);
                client_read(client_sock, msg, MSGLEN);
                printk("read from client!\n");
                if(msg[0] == 'C' && msg[1] == 'M' && msg[2] == 'D' && msg[5] == MSGENDFLAG)
                {
                    // 执行业务逻辑
                    if(do_cmd(msg[3], msg[4], client_sock) == -1)
                        break;
                }
                else
                {
                    printk("cmd error: %s", msg);
                }
            }
            /* 重置看门狗的计时器，重新开始计时 */
            wdt_feed(WDT_DEVICE_0);
        }
        /* 重置看门狗的计时器，重新开始计时 */
        wdt_feed(WDT_DEVICE_0);
    }

    return 0;
}