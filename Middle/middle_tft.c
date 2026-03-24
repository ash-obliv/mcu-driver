#include "middle_tft.h"
#include "main.h"
#include "spi.h"

//========== GPIO初始化函数 ==========
static void _GPIO_Init(void)
{
    // HAL库的GPIO初始化通常在MX_GPIO_Init()函数中完成
    return;
}

//========== 硬件SPI写一字节函数 ==========
static void _SPI_WriteByte(uint8_t byte)
{
    HAL_SPI_Transmit(&hspi1, &byte, 1, HAL_MAX_DELAY);
}

//========== 延时函数 ==========
static void _Delay_ms(uint32_t ms)
{
    // 使用HAL库提供的延时函数
    HAL_Delay(ms);
}

//========== DC引脚控制 ==========
static void _Set_DC_Pin(uint8_t level)
{
    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

//========== CS引脚控制 ==========
static void _Set_CS_Pin(uint8_t level)
{
    HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

//========== RST引脚控制 ==========
static void _Set_RST_Pin(uint8_t level)
{
    HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

//========== 背光控制 ==========
static void _Set_BLK_Pin(uint8_t level)
{
    // 背光这里默认高电平 如果需要加上gpio控制就可以
    return;
}

//========== 驱动程序结构体 ==========
static const TFT_Driver_t s_tft_driver = {
    .gpio_init = _GPIO_Init,
    .spi_write_byte = _SPI_WriteByte,
    .delay_ms = _Delay_ms,
    .set_dc_pin = _Set_DC_Pin,
    .set_cs_pin = _Set_CS_Pin,
    .set_rst_pin = _Set_RST_Pin,
    .set_blk_pin = _Set_BLK_Pin,
};

//========== 初始化函数 ==========
TFT_Status_t BSP_TFT_Init(void)
{
    // 注册驱动
    TFT_RegisterDriver(&s_tft_driver);

    // 初始化屏幕
    TFT_Status_t status = TFT_Init();

    if (status != TFT_OK)
    {
        return status;
    }

    // 清屏为黑色
    TFT_Clear(BLACK);

    return TFT_OK;
}
