#ifndef __MPU6050_H
#define __MPU6050_H

#include <stdint.h>
#include <stdbool.h>

#define MPU6050_I2C_ADDR_AD0_LOW  0x68
#define MPU6050_I2C_ADDR_AD0_HIGH 0x69

typedef enum
{
	MPU6050_OK = 0,
	MPU6050_ERR_DRIVER_NOT_REGISTERED = -1,
	MPU6050_ERR_NULL_FUNC_PTR = -2,
	MPU6050_ERR_I2C = -3,
	MPU6050_ERR_BAD_PARAM = -4,
	MPU6050_ERR_NOT_INITIALIZED = -5,
	MPU6050_ERR_WHO_AM_I = -6,
} MPU6050_Status_t;

typedef enum
{
	MPU6050_ACCEL_RANGE_2G = 0,
	MPU6050_ACCEL_RANGE_4G = 1,
	MPU6050_ACCEL_RANGE_8G = 2,
	MPU6050_ACCEL_RANGE_16G = 3,
} MPU6050_AccelRange_t;

typedef enum
{
	MPU6050_GYRO_RANGE_250DPS = 0,
	MPU6050_GYRO_RANGE_500DPS = 1,
	MPU6050_GYRO_RANGE_1000DPS = 2,
	MPU6050_GYRO_RANGE_2000DPS = 3,
} MPU6050_GyroRange_t;

typedef struct
{
	int16_t ax;
	int16_t ay;
	int16_t az;
	int16_t gx;
	int16_t gy;
	int16_t gz;
	int16_t temp_raw;

	float ax_g;
	float ay_g;
	float az_g;
	float gx_dps;
	float gy_dps;
	float gz_dps;
	float temp_c;
} MPU6050_Data_t;

typedef struct
{
	bool (*i2c_write_reg)(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
	bool (*i2c_read_regs)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
	void (*delay_ms)(uint32_t ms);
} MPU6050_Driver_t;

void MPU6050_RegisterDriver(const MPU6050_Driver_t *driver);
bool MPU6050_IsDriverRegistered(void);

MPU6050_Status_t MPU6050_Init(void);
MPU6050_Status_t MPU6050_Deinit(void);

MPU6050_Status_t MPU6050_SetAddress(uint8_t i2c_addr_7bit);
uint8_t MPU6050_GetAddress(void);

MPU6050_Status_t MPU6050_SetAccelRange(MPU6050_AccelRange_t range);
MPU6050_Status_t MPU6050_SetGyroRange(MPU6050_GyroRange_t range);

MPU6050_Status_t MPU6050_ReadWhoAmI(uint8_t *who_am_i);
MPU6050_Status_t MPU6050_ReadRaw(int16_t *ax, int16_t *ay, int16_t *az,
								 int16_t *gx, int16_t *gy, int16_t *gz,
								 int16_t *temp_raw);
MPU6050_Status_t MPU6050_ReadAll(MPU6050_Data_t *out);
MPU6050_Status_t MPU6050_ReadTemperature(float *temp_c);

#endif
