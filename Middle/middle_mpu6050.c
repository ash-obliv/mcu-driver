#include "middle_mpu6050.h"
#include "main.h"

// Override these weak functions in your hardware layer when porting.
__weak bool BSP_MPU6050_I2C_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
    (void)dev_addr;
    (void)reg_addr;
    (void)data;
    return false;
}

__weak bool BSP_MPU6050_I2C_ReadRegs(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    (void)dev_addr;
    (void)reg_addr;
    (void)data;
    (void)len;
    return false;
}

__weak void BSP_MPU6050_DelayMs(uint32_t ms)
{
    HAL_Delay(ms);
}

static bool _I2C_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
    return BSP_MPU6050_I2C_WriteReg(dev_addr, reg_addr, data);
}

static bool _I2C_ReadRegs(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    return BSP_MPU6050_I2C_ReadRegs(dev_addr, reg_addr, data, len);
}

static void _DelayMs(uint32_t ms)
{
    BSP_MPU6050_DelayMs(ms);
}

static const MPU6050_Driver_t s_mpu_driver = {
    .i2c_write_reg = _I2C_WriteReg,
    .i2c_read_regs = _I2C_ReadRegs,
    .delay_ms = _DelayMs,
};

MPU6050_Status_t BSP_MPU6050_Init(void)
{
    MPU6050_RegisterDriver(&s_mpu_driver);
    return MPU6050_Init();
}

MPU6050_Status_t BSP_MPU6050_ReadAll(MPU6050_Data_t *out)
{
    return MPU6050_ReadAll(out);
}
