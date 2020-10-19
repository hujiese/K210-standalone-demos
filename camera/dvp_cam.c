#include <stdio.h>
#include "dvp_cam.h"
#include "dvp.h"
#include "plic.h"
#include "sysctl.h"
#include "iomem.h"

uint32_t *display_buf = NULL;
uint32_t display_buf_addr = 0;
volatile uint8_t g_dvp_finish_flag;

/* dvp中断回调函数 */
static int on_dvp_irq_cb(void *ctx)
{
    /* 读取DVP中断状态，如果完成则刷新显示地址的数据，并清除中断标志，否则读取摄像头数据*/
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        dvp_set_display_addr((uint32_t)display_buf_addr);
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    }
    else
    {
        if (g_dvp_finish_flag == 0)
            dvp_start_convert();
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }
    return 0;
}

/* dvp初始化 */
void dvp_cam_init(void)
{
    /* DVP初始化，设置sccb的寄存器长度为8bit */
    dvp_init(8);
    /* 设置输入时钟为24000000*/
    dvp_set_xclk_rate(24000000);
    /* 使能突发传输模式 */
    dvp_enable_burst();
    /* 关闭AI输出模式，使能显示模式 */
    dvp_set_output_enable(DVP_OUTPUT_AI, 0);
    dvp_set_output_enable(DVP_OUTPUT_DISPLAY, 1);
    /* 设置输出格式为RGB */
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    /* 设置输出像素大小为320*240 */
    dvp_set_image_size(CAM_WIDTH_PIXEL, CAM_HIGHT_PIXEL);

    /* 设置DVP的显示地址参数和中断 */
    display_buf = (uint32_t*)iomem_malloc(CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL * 2);
    display_buf_addr = display_buf;
    dvp_set_display_addr((uint32_t)display_buf_addr);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();
}

void dvp_cam_set_irq(void)
{
    /* DVP 中断配置：中断优先级，中断回调，使能DVP中断 */
    printf("DVP interrupt config\r\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, on_dvp_irq_cb, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    /* 清除DVP中断位 */
    g_dvp_finish_flag = 0;
    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
}
