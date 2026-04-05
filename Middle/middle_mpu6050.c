#include "middle_mpu6050.h"
#include "main.h"

/** 
 * 这个函数需要自己完善 因为要确定设备的从机地址
 * @param dev_addr I2C设备地址（7位地址，不包含读写位）
 * @param reg_addr 寄存器地址
 * @param data 要写入的数据
 * @return true表示写入成功，false表示失败
 */
__weak bool BSP_MPU6050_I2C_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
    (void)dev_addr;
    (void)reg_addr;
    (void)data;
    return false;

    // example implementation using HAL I2C (uncomment and adjust as needed):
    // uint8_t buf[2] = {reg_addr, data};
    // return HAL_I2C_Master_Transmit(&hi2c1, dev_addr << 1, buf, 2, HAL_MAX_DELAY) == HAL_OK;
}


/** 
 * 这个函数需要自己完善 因为要确定设备的从机地址
 * @param dev_addr I2C设备地址（7位地址，不包含读写位）
 * @param reg_addr 寄存器地址
 * @param data 要读取的数据
 * @param len 要读取的数据长度
 * @return true表示写入成功，false表示失败
 */
__weak bool BSP_MPU6050_I2C_ReadRegs(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    (void)dev_addr;
    (void)reg_addr;
    (void)data;
    (void)len;
    return false;

    // example implementation using HAL I2C (uncomment and adjust as needed):
    // if (HAL_I2C_Master_Transmit(&hi2c1, dev_addr << 1, &reg_addr, 1, HAL_MAX_DELAY) != HAL_OK)
    //     return false;
    // return HAL_I2C_Master_Receive(&hi2c1, dev_addr << 1, data, len, HAL_MAX_DELAY) == HAL_OK;
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
