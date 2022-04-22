#ifndef BSP_USART_H
#define BSP_USART_H
#include "struct_typedef.h"

extern void usart1_tx_dma_init(void);
extern void usart1_tx_dma_enable(uint8_t *data, uint16_t len);

extern void laser_on(void);
extern void laser_off(void);

extern void pwm_on(void);
extern void pwm_off(void);

#endif
