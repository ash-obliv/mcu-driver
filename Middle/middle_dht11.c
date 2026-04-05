#include "middle_dht11.h"
#include "main.h"

//========== DHT11 硬件引脚定义 ==========
// 请根据你的实际硬件修改这些定义
// 示例（假设 DHT11 数据线连接到 PA2）：
#define DHT11_DATA_PIN         GPIO_PIN_2
#define DHT11_DATA_GPIO_PORT   GPIOA

//========== GPIO 初始化函数 ==========
static void _GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 确保时钟已启用（通常已在 MX_GPIO_Init 中启用）
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 初始化为输入模式（浮空输入或上拉输入）
    GPIO_InitStruct.Pin = DHT11_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // 或改为 GPIO_PULLUP，取决于你的硬件电路
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(DHT11_DATA_GPIO_PORT, &GPIO_InitStruct);

    // 初始状态：释放引脚（高电平）
    // 这会让外部上拉电阻将引脚拉高
}

//========== 设置引脚为输出模式（推挽）==========
static void _Pin_Set_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(DHT11_DATA_GPIO_PORT, &GPIO_InitStruct);
}

//========== 设置引脚为输入模式 ==========
static void _Pin_Set_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // 浮空输入，让外部上拉电阻工作
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(DHT11_DATA_GPIO_PORT, &GPIO_InitStruct);
}

//========== 写引脚电平 ==========
static void _Pin_Write(uint8_t level)
{
    HAL_GPIO_WritePin(DHT11_DATA_GPIO_PORT, DHT11_DATA_PIN, 
                      level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

//========== 读引脚电平 ==========
static uint8_t _Pin_Read(void)
{
    return HAL_GPIO_ReadPin(DHT11_DATA_GPIO_PORT, DHT11_DATA_PIN) == GPIO_PIN_SET ? 1 : 0;
}

//========== 微秒延时 ==========
static void _Delay_Us(uint32_t us)
{
    // 使用 DWT（Data Watchpoint and Trace）实现精确的微秒延时
    // 假设 SystemCoreClock = 72MHz
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    
    while ((DWT->CYCCNT - start) < ticks);
}

//========== 驱动程序结构体 ==========
static const DHT11_Driver_t s_dht11_driver = {
    .gpio_init = _GPIO_Init,
    .pin_set_output = _Pin_Set_Output,
    .pin_set_input = _Pin_Set_Input,
    .pin_write = _Pin_Write,
    .pin_read = _Pin_Read,
    .delay_us = _Delay_Us,
};

//========== 初始化函数 ==========
DHT11_Status_t BSP_DHT11_Init(void)
{
    // 初始化 DWT 用于微秒延时
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // 注册驱动
    DHT11_RegisterDriver(&s_dht11_driver);

    // 初始化 DHT11
    DHT11_Status_t status = DHT11_Init();

    if (status != DHT11_OK)
    {
        return status;
    }

    return DHT11_OK;
}

//========== 便捷接口 ==========
DHT11_Status_t BSP_DHT11_ReadData(DHT11_Data_t *data)
{
    return DHT11_ReadData(data);
}

DHT11_Status_t BSP_DHT11_GetTemperature(float *temp)
{
    return DHT11_GetTemperature(temp);
}

DHT11_Status_t BSP_DHT11_GetHumidity(float *humidity)
{
    return DHT11_GetHumidity(humidity);
}