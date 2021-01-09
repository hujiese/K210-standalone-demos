#include <stdio.h>
#include <unistd.h>
#include "wdt.h"
#include "sysctl.h"

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
    /* 打印系统启动信息 */
    printf("system start!\n");
    /* 记录feed的次数 */
    int times = 0;

    /* 系统中断初始化 */
    plic_init();
    sysctl_enable_irq();

    /* 启动看门狗，设置超时时间为2秒后调用中断函数wdt0_irq_cb */
    int timeout = wdt_init(WDT_DEVICE_0, 2000, wdt0_irq_cb, NULL);

    /* 打印看门狗实际超时的时间 */
    printf("wdt timeout is %d ms!\n", timeout);
    
    while(1)
    {
        sleep(1);
        if(times++ < 5)
        {
            /* 打印feed的次数 */
            printf("wdt_feed %d times!\n", times);

            /* 重置看门狗的计时器，重新开始计时 */
            wdt_feed(WDT_DEVICE_0);
        }
    }
}

