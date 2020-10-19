#ifndef _L_CONV_H_
#define _L_CONV_H_

typedef enum _enum_convolution_kernel_t
{
    CONV_1_1=1,
    CONV_3_3=9,

} convolution_kernel_t;

void conv_init(kpu_task_t *task, convolution_kernel_t conv_x_x, float *conv_data);
void conv_run(kpu_task_t *task, uint8_t *img_src, uint8_t *img_dst, plic_irq_callback_t callback);

#endif
