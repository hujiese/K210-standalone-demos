#ifndef _DVP_CAM_H_
#define _DVP_CAM_H_

#include <stdint.h>

#define CAM_WIDTH_PIXEL        (320)
#define CAM_HIGHT_PIXEL        (240)

extern volatile uint8_t g_ram_mux;
extern uint32_t display_buf_addr1;
extern uint32_t display_buf_addr2;
extern volatile uint8_t g_dvp_finish_flag;

void dvp_cam_init(void);
void dvp_cam_set_irq(void);

#endif /* _DVP_CAM_H_ */
