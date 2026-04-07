#ifndef __MIDDLE_OLED_H
#define __MIDDLE_OLED_H

#include <stdint.h>

void BSP_OLED_Init(void);

void BSP_OLED_I2C_WriteByte(uint8_t data);
void BSP_OLED_I2C_WriteBytes(const uint8_t *data, uint16_t count);

#endif
