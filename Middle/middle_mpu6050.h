#ifndef __MIDDLE_MPU6050_H
#define __MIDDLE_MPU6050_H

#include "mpu6050.h"

bool BSP_MPU6050_I2C_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
bool BSP_MPU6050_I2C_ReadRegs(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
void BSP_MPU6050_DelayMs(uint32_t ms);

MPU6050_Status_t BSP_MPU6050_Init(void);
MPU6050_Status_t BSP_MPU6050_ReadAll(MPU6050_Data_t *out);

#endif
