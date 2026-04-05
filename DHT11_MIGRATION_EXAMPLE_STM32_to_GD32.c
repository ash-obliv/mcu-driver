/**
 * ============================================================
 * DHT11 迁移示例：STM32F103 → GD32F103
 * ============================================================
 * 
 * 本文件展示完整的迁移过程。
 * 只需改这个文件中标记为【重点】的部分！
 * 
 * 文件信息：
 *   原始：Middle/middle_dht11.c (STM32 版本)
 *   目标：Middle/middle_dht11_gd32.c (GD32 版本)
 *   改动量：6 个函数，约 50 行代码
 */

/**
 * ========== STM32F103 版本 ==========
 */
#if 0  // 原始 STM32 版本（参考）

#include "middle_dht11.h"
#include "main.h"

// 【重点】STM32 使用 HAL 库，包含头文件
#include "stm32f1xx_hal.h"

static void _GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 【重点】启用时钟：STM32 HAL 方式
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void _Pin_Set_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // 【重点】推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void _Pin_Set_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  // 【重点】输入
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void _Pin_Write(uint8_t level)
{
    // 【重点】写引脚：STM32 HAL 方式
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, 
                      level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t _Pin_Read(void)
{
    // 【重点】读引脚：STM32 HAL 方式
    return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_SET ? 1 : 0;
}

static void _Delay_Us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

#endif

/**
 * ========== GD32F103 版本 ==========
 * 
 * 迁移步骤：
 * 1. 包含头文件改为 gd32f1xx.h
 * 2. 时钟启用改为 GD32 API
 * 3. GPIO 初始化改为 GD32 API
 * 4. 引脚操作改为 GD32 API
 * 5. 延时函数保持不变（DWT 相同）
 */

#include "middle_dht11.h"
#include "main.h"

// 【改 1】包含头文件：从 STM32 改为 GD32
#include "gd32f1xx.h"
// #include "stm32f1xx_hal.h"  // 删除此行

// 【提示】GD32 GPIO 端口定义
// GPIOA, GPIOB, GPIOC, ... (与 STM32 相同)
// GPIO_PIN_0, GPIO_PIN_1, ... (与 STM32 相同)

//========== DHT11 硬件引脚定义 ==========
#define DHT11_DATA_PIN         GPIO_PIN_2
#define DHT11_DATA_GPIO_PORT   GPIOA

//========== GPIO 初始化函数 ==========
static void _GPIO_Init(void)
{
    // 【改 2】启用时钟：从 HAL 改为 GD32 原生 API
    // STM32: __HAL_RCC_GPIOA_CLK_ENABLE();
    // GD32:  rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOA);
    
    // 【改 3】GPIO 初始化：从 HAL 改为 GD32 API
    // STM32 使用结构体，GD32 使用函数调用
    // gpio_init(port, mode, speed, pin)
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
}

//========== 设置引脚为输出模式（推挽）==========
static void _Pin_Set_Output(void)
{
    // 【改 3】GPIO 初始化：从 HAL 改为 GD32 API
    // STM32: GPIO_MODE_OUTPUT_PP
    // GD32:  GPIO_MODE_OUT_PP
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
}

//========== 设置引脚为输入模式 ==========
static void _Pin_Set_Input(void)
{
    // 【改 3】GPIO 初始化：从 HAL 改为 GD32 API
    // STM32: GPIO_MODE_INPUT
    // GD32:  GPIO_MODE_IN_FLOATING
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
}

//========== 写引脚电平 ==========
static void _Pin_Write(uint8_t level)
{
    // 【改 4】写引脚：从 HAL 改为 GD32 API
    // STM32: HAL_GPIO_WritePin(port, pin, level)
    // GD32:  gpio_bit_write(port, pin, level)
    gpio_bit_write(GPIOA, GPIO_PIN_2, (bit_status)level);
}

//========== 读引脚电平 ==========
static uint8_t _Pin_Read(void)
{
    // 【改 4】读引脚：从 HAL 改为 GD32 API
    // STM32: HAL_GPIO_ReadPin(port, pin) 返回值是 GPIO_PIN_SET/RESET
    // GD32:  gpio_input_bit_get(port, pin) 返回值是 0 或 1
    return gpio_input_bit_get(GPIOA, GPIO_PIN_2);  // 已是 0 或 1，无需转换
}

//========== 微秒延时 ==========
static void _Delay_Us(uint32_t us)
{
    // 【不改】延时函数：DWT 定时器在 Cortex-M3 上相同
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
    // 【不改】DWT 初始化：相同
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // 【不改】驱动注册和初始化：相同
    DHT11_RegisterDriver(&s_dht11_driver);
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

/**
 * ============================================================
 * 对比总结
 * ============================================================
 * 
 * ╔════════════════════╦══════════════════╦════════════════════╗
 * ║     操作项         ║   STM32 (HAL)    ║  GD32 (原生 API)   ║
 * ╠════════════════════╬══════════════════╬════════════════════╣
 * ║ 包含头文件         ║ stm32f1xx_hal.h  ║ gd32f1xx.h         ║
 * ║ 启用时钟           ║ __HAL_RCC_*      ║ rcu_periph_*       ║
 * ║ GPIO 初始化        ║ HAL_GPIO_Init()  ║ gpio_init()        ║
 * ║ 输出模式           ║ GPIO_MODE_OUT_PP ║ GPIO_MODE_OUT_PP   ║
 * ║ 输入模式           ║ GPIO_MODE_INPUT  ║ GPIO_MODE_IN_*     ║
 * ║ 写引脚             ║ HAL_GPIO_Write*  ║ gpio_bit_write()   ║
 * ║ 读引脚             ║ HAL_GPIO_Read*   ║ gpio_input_bit_*   ║
 * ║ 延时               ║ DWT 定时器       ║ DWT 定时器（相同） ║
 * ║ 总改动代码行数     ║      0 行        ║      30 行         ║
 * ║ 总改动时间         ║      0 分        ║      10 分         ║
 * ╚════════════════════╩══════════════════╩════════════════════╝
 * 
 */

/**
 * ============================================================
 * 验证检查清单
 * ============================================================
 */

/*
编译检查：
  ✓ 编译无误
  ✓ 没有 HAL 相关的符号未定义错误
  
功能检查：
  ✓ BSP_DHT11_Init() 返回 DHT11_OK
  ✓ DHT11_ReadData() 能读到数据
  ✓ 读到的数据校验和正确
  ✓ 温度值在合理范围 (-40~125°C)
  ✓ 湿度值在合理范围 (0~100%)
  
时序检查（可选，用逻辑分析仪）：
  ✓ 起始信号：主机拉低 300ms
  ✓ 响应信号：DHT11 拉低再拉高
  ✓ 数据位：0 约 23μs，1 约 68μs
  
性能检查：
  ✓ 读取一次完整数据约 5-10ms
  ✓ 支持 2-3 秒读一次（DHT11 刷新周期）

迁移成功标志：
  ✓ 设备在 30 分钟内完成移植
  ✓ 代码改动少于 50 行
  ✓ 上层应用代码零改动
  ✓ 可以快速移植到更多平台
*/
