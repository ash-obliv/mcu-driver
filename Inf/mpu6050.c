#include "mpu6050.h"
#include <stddef.h>

#define MPU6050_REG_SMPLRT_DIV   0x19
#define MPU6050_REG_CONFIG       0x1A
#define MPU6050_REG_GYRO_CONFIG  0x1B
#define MPU6050_REG_ACCEL_CONFIG 0x1C
#define MPU6050_REG_INT_ENABLE   0x38
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_TEMP_OUT_H   0x41
#define MPU6050_REG_GYRO_XOUT_H  0x43
#define MPU6050_REG_PWR_MGMT_1   0x6B
#define MPU6050_REG_WHO_AM_I     0x75

static const MPU6050_Driver_t *g_mpu_driver = NULL;
static bool g_mpu_initialized = false;
static uint8_t g_mpu_addr = MPU6050_I2C_ADDR_AD0_LOW;
static MPU6050_AccelRange_t g_accel_range = MPU6050_ACCEL_RANGE_2G;
static MPU6050_GyroRange_t g_gyro_range = MPU6050_GYRO_RANGE_250DPS;

static MPU6050_Status_t _MPU6050_VerifyDriver(void)
{
	if (g_mpu_driver == NULL)
	{
		return MPU6050_ERR_DRIVER_NOT_REGISTERED;
	}

	if (g_mpu_driver->i2c_write_reg == NULL ||
		g_mpu_driver->i2c_read_regs == NULL ||
		g_mpu_driver->delay_ms == NULL)
	{
		return MPU6050_ERR_NULL_FUNC_PTR;
	}

	return MPU6050_OK;
}

static MPU6050_Status_t _MPU6050_WriteReg(uint8_t reg, uint8_t val)
{
	if (!g_mpu_driver->i2c_write_reg(g_mpu_addr, reg, val))
	{
		return MPU6050_ERR_I2C;
	}
	return MPU6050_OK;
}

static MPU6050_Status_t _MPU6050_ReadRegs(uint8_t reg, uint8_t *buf, uint16_t len)
{
	if (buf == NULL || len == 0)
	{
		return MPU6050_ERR_BAD_PARAM;
	}

	if (!g_mpu_driver->i2c_read_regs(g_mpu_addr, reg, buf, len))
	{
		return MPU6050_ERR_I2C;
	}
	return MPU6050_OK;
}

static float _MPU6050_GetAccelLSBPerG(MPU6050_AccelRange_t range)
{
	switch (range)
	{
	case MPU6050_ACCEL_RANGE_2G:
		return 16384.0f;
	case MPU6050_ACCEL_RANGE_4G:
		return 8192.0f;
	case MPU6050_ACCEL_RANGE_8G:
		return 4096.0f;
	case MPU6050_ACCEL_RANGE_16G:
		return 2048.0f;
	default:
		return 16384.0f;
	}
}

static float _MPU6050_GetGyroLSBPerDps(MPU6050_GyroRange_t range)
{
	switch (range)
	{
	case MPU6050_GYRO_RANGE_250DPS:
		return 131.0f;
	case MPU6050_GYRO_RANGE_500DPS:
		return 65.5f;
	case MPU6050_GYRO_RANGE_1000DPS:
		return 32.8f;
	case MPU6050_GYRO_RANGE_2000DPS:
		return 16.4f;
	default:
		return 131.0f;
	}
}

void MPU6050_RegisterDriver(const MPU6050_Driver_t *driver)
{
	if (driver == NULL)
	{
		return;
	}
	g_mpu_driver = driver;
}

bool MPU6050_IsDriverRegistered(void)
{
	return g_mpu_driver != NULL;
}

MPU6050_Status_t MPU6050_SetAddress(uint8_t i2c_addr_7bit)
{
	if (i2c_addr_7bit != MPU6050_I2C_ADDR_AD0_LOW &&
		i2c_addr_7bit != MPU6050_I2C_ADDR_AD0_HIGH)
	{
		return MPU6050_ERR_BAD_PARAM;
	}

	g_mpu_addr = i2c_addr_7bit;
	return MPU6050_OK;
}

uint8_t MPU6050_GetAddress(void)
{
	return g_mpu_addr;
}

MPU6050_Status_t MPU6050_Init(void)
{
	MPU6050_Status_t status = _MPU6050_VerifyDriver();
	if (status != MPU6050_OK)
	{
		return status;
	}

	status = _MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x80);
	if (status != MPU6050_OK)
	{
		return status;
	}
	g_mpu_driver->delay_ms(100);

	status = _MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x01);
	if (status != MPU6050_OK)
	{
		return status;
	}
	g_mpu_driver->delay_ms(10);

	status = _MPU6050_WriteReg(MPU6050_REG_SMPLRT_DIV, 0x07);
	if (status != MPU6050_OK)
	{
		return status;
	}

	status = _MPU6050_WriteReg(MPU6050_REG_CONFIG, 0x06);
	if (status != MPU6050_OK)
	{
		return status;
	}

	status = MPU6050_SetGyroRange(MPU6050_GYRO_RANGE_250DPS);
	if (status != MPU6050_OK)
	{
		return status;
	}

	status = MPU6050_SetAccelRange(MPU6050_ACCEL_RANGE_2G);
	if (status != MPU6050_OK)
	{
		return status;
	}

	status = _MPU6050_WriteReg(MPU6050_REG_INT_ENABLE, 0x00);
	if (status != MPU6050_OK)
	{
		return status;
	}

	uint8_t who_am_i = 0;
	status = MPU6050_ReadWhoAmI(&who_am_i);
	if (status != MPU6050_OK)
	{
		return status;
	}

	if (who_am_i != 0x68)
	{
		return MPU6050_ERR_WHO_AM_I;
	}

	g_mpu_initialized = true;
	return MPU6050_OK;
}

MPU6050_Status_t MPU6050_Deinit(void)
{
	if (!g_mpu_initialized)
	{
		return MPU6050_OK;
	}

	MPU6050_Status_t status = _MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x40);
	if (status != MPU6050_OK)
	{
		return status;
	}

	g_mpu_initialized = false;
	return MPU6050_OK;
}

MPU6050_Status_t MPU6050_SetAccelRange(MPU6050_AccelRange_t range)
{
	if (range > MPU6050_ACCEL_RANGE_16G)
	{
		return MPU6050_ERR_BAD_PARAM;
	}

	MPU6050_Status_t status = _MPU6050_WriteReg(MPU6050_REG_ACCEL_CONFIG, (uint8_t)(range << 3));
	if (status != MPU6050_OK)
	{
		return status;
	}

	g_accel_range = range;
	return MPU6050_OK;
}

MPU6050_Status_t MPU6050_SetGyroRange(MPU6050_GyroRange_t range)
{
	if (range > MPU6050_GYRO_RANGE_2000DPS)
	{
		return MPU6050_ERR_BAD_PARAM;
	}

	MPU6050_Status_t status = _MPU6050_WriteReg(MPU6050_REG_GYRO_CONFIG, (uint8_t)(range << 3));
	if (status != MPU6050_OK)
	{
		return status;
	}

	g_gyro_range = range;
	return MPU6050_OK;
}

MPU6050_Status_t MPU6050_ReadWhoAmI(uint8_t *who_am_i)
{
	if (who_am_i == NULL)
	{
		return MPU6050_ERR_BAD_PARAM;
	}

	return _MPU6050_ReadRegs(MPU6050_REG_WHO_AM_I, who_am_i, 1);
}

MPU6050_Status_t MPU6050_ReadRaw(int16_t *ax, int16_t *ay, int16_t *az,
								 int16_t *gx, int16_t *gy, int16_t *gz,
								 int16_t *temp_raw)
{
	if (!g_mpu_initialized)
	{
		return MPU6050_ERR_NOT_INITIALIZED;
	}

	if (ax == NULL || ay == NULL || az == NULL ||
		gx == NULL || gy == NULL || gz == NULL || temp_raw == NULL)
	{
		return MPU6050_ERR_BAD_PARAM;
	}

	uint8_t buf[14] = {0};
	MPU6050_Status_t status = _MPU6050_ReadRegs(MPU6050_REG_ACCEL_XOUT_H, buf, sizeof(buf));
	if (status != MPU6050_OK)
	{
		return status;
	}

	*ax = (int16_t)((buf[0] << 8) | buf[1]);
	*ay = (int16_t)((buf[2] << 8) | buf[3]);
	*az = (int16_t)((buf[4] << 8) | buf[5]);
	*temp_raw = (int16_t)((buf[6] << 8) | buf[7]);
	*gx = (int16_t)((buf[8] << 8) | buf[9]);
	*gy = (int16_t)((buf[10] << 8) | buf[11]);
	*gz = (int16_t)((buf[12] << 8) | buf[13]);

	return MPU6050_OK;
}

MPU6050_Status_t MPU6050_ReadTemperature(float *temp_c)
{
	if (temp_c == NULL)
	{
		return MPU6050_ERR_BAD_PARAM;
	}

	if (!g_mpu_initialized)
	{
		return MPU6050_ERR_NOT_INITIALIZED;
	}

	uint8_t buf[2] = {0};
	MPU6050_Status_t status = _MPU6050_ReadRegs(MPU6050_REG_TEMP_OUT_H, buf, 2);
	if (status != MPU6050_OK)
	{
		return status;
	}

	int16_t raw = (int16_t)((buf[0] << 8) | buf[1]);
	*temp_c = ((float)raw / 340.0f) + 36.53f;
	return MPU6050_OK;
}

MPU6050_Status_t MPU6050_ReadAll(MPU6050_Data_t *out)
{
	if (out == NULL)
	{
		return MPU6050_ERR_BAD_PARAM;
	}

	int16_t ax = 0, ay = 0, az = 0;
	int16_t gx = 0, gy = 0, gz = 0;
	int16_t temp_raw = 0;

	MPU6050_Status_t status = MPU6050_ReadRaw(&ax, &ay, &az, &gx, &gy, &gz, &temp_raw);
	if (status != MPU6050_OK)
	{
		return status;
	}

	float accel_lsb_per_g = _MPU6050_GetAccelLSBPerG(g_accel_range);
	float gyro_lsb_per_dps = _MPU6050_GetGyroLSBPerDps(g_gyro_range);

	out->ax = ax;
	out->ay = ay;
	out->az = az;
	out->gx = gx;
	out->gy = gy;
	out->gz = gz;
	out->temp_raw = temp_raw;

	out->ax_g = (float)ax / accel_lsb_per_g;
	out->ay_g = (float)ay / accel_lsb_per_g;
	out->az_g = (float)az / accel_lsb_per_g;
	out->gx_dps = (float)gx / gyro_lsb_per_dps;
	out->gy_dps = (float)gy / gyro_lsb_per_dps;
	out->gz_dps = (float)gz / gyro_lsb_per_dps;
	out->temp_c = ((float)temp_raw / 340.0f) + 36.53f;

	return MPU6050_OK;
}