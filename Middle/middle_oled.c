#include "middle_oled.h"
#include "i2c.h"
#include "OLED.h"

#define OLED_I2C_ADDR (0x3C << 1)

void BSP_OLED_I2C_WriteBytes(const uint8_t *data, uint16_t count)
{
	if (data == NULL || count == 0)
	{
		return;
	}

	HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDR, (uint8_t *)data, count, HAL_MAX_DELAY);
}

void BSP_OLED_I2C_WriteByte(uint8_t data)
{
	BSP_OLED_I2C_WriteBytes(&data, 1);
}

void BSP_OLED_Init(void)
{
	OLED_RegisterDriver(BSP_OLED_I2C_WriteBytes);
	OLED_Init();
}
