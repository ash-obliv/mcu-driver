#include "dht11.h"
#include <string.h>

//========== 全局驱动指针 ==========
static const DHT11_Driver_t *g_dht11_driver = NULL;
static bool g_dht11_initialized = false;

//========== 驱动管理 ==========
void DHT11_RegisterDriver(const DHT11_Driver_t *driver)
{
    if (driver == NULL)
    {
        return;
    }
    g_dht11_driver = driver;
}

bool DHT11_IsDriverRegistered(void)
{
    return g_dht11_driver != NULL;
}

//========== 驱动函数验证 ==========
static DHT11_Status_t DHT11_VerifyDriver(void)
{
    if (g_dht11_driver == NULL)
    {
        return DHT11_ERR_DRIVER_NOT_REGISTERED;
    }

    if (g_dht11_driver->gpio_init == NULL ||
        g_dht11_driver->pin_set_output == NULL ||
        g_dht11_driver->pin_set_input == NULL ||
        g_dht11_driver->pin_write == NULL ||
        g_dht11_driver->pin_read == NULL ||
        g_dht11_driver->delay_us == NULL)
    {
        return DHT11_ERR_NULL_FUNC_PTR;
    }

    return DHT11_OK;
}

//========== 初始化 ==========
DHT11_Status_t DHT11_Init(void)
{
    DHT11_Status_t status = DHT11_VerifyDriver();
    if (status != DHT11_OK)
    {
        return status;
    }

    // 调用硬件初始化
    if (g_dht11_driver->gpio_init != NULL)
    {
        g_dht11_driver->gpio_init();
    }

    g_dht11_initialized = true;
    return DHT11_OK;
}

DHT11_Status_t DHT11_Uninit(void)
{
    g_dht11_initialized = false;
    return DHT11_OK;
}

//========== DHT11 读取通信协议 ==========
/**
 * DHT11 协议分析：
 * 1. 主机拉低200-500ms（发送起始信号）
 * 2. 主机释放（让下拉器件拉高），等待DHT11响应
 * 3. DHT11 拉低 83μs，再拉高 87μs（表示已收到）
 * 4. DHT11 发送 40 bit 数据：
 *    - 拉低 54μs
 *    - 如果是0：拉高 23-27μs
 *    - 如果是1：拉高 68-74μs
 * 5. 数据格式：湿度整数(8bit) 湿度小数(8bit) 温度整数(8bit) 温度小数(8bit) 校验和(8bit)
 */

// 等待引脚电平变化（用于超时控制）
static DHT11_Status_t _Wait_Pin_Edge(uint8_t target_level, uint32_t timeout_us)
{
    uint32_t start = 0;
    // 简单的超时计数（实际应该用定时器，这里用循环估算）
    for (uint32_t i = 0; i < timeout_us * 2; i++)
    {
        if (g_dht11_driver->pin_read() == target_level)
        {
            return DHT11_OK;
        }
    }
    return DHT11_ERR_TIMEOUT;
}

// 测量脉冲宽度
static uint32_t _Measure_Pulse_Width(uint8_t start_level)
{
    uint32_t width = 0;
    uint8_t current_level = start_level;
    
    // 等待电平反转
    while (g_dht11_driver->pin_read() == start_level && width < 250)
    {
        g_dht11_driver->delay_us(1);
        width++;
    }
    
    return width;
}

/**
 * DHT11_ReadData - 读取DHT11温湿度数据
 * @data: 指向 DHT11_Data_t 结构的指针，用于存储读取的数据
 * 
 * 返回值：
 *   DHT11_OK: 成功
 *   其他: 失败原因
 */
DHT11_Status_t DHT11_ReadData(DHT11_Data_t *data)
{
    if (data == NULL)
    {
        return DHT11_ERR_TIMEOUT;
    }

    if (!g_dht11_initialized)
    {
        return DHT11_ERR_INIT_FAILED;
    }

    uint8_t dht11_data[5] = {0};
    uint32_t pulse_width;

    //========== 1. 发送起始信号 ==========
    // 设置为输出，拉低 PIN 200-500ms（这里用300ms）
    g_dht11_driver->pin_set_output();
    g_dht11_driver->pin_write(0);
    g_dht11_driver->delay_us(300000);  // 300ms
    
    // 释放引脚（输入模式，让上拉器件拉高）
    g_dht11_driver->pin_set_input();
    g_dht11_driver->delay_us(40);  // 等待40μs

    //========== 2. 等待DHT11响应 ==========
    // 等待引脚被DHT11拉低（超时 100μs）
    if (_Wait_Pin_Edge(0, 100) != DHT11_OK)
    {
        data->valid = false;
        return DHT11_ERR_NO_RESPONSE;
    }

    // 等待DHT11释放（超时 100μs）
    if (_Wait_Pin_Edge(1, 100) != DHT11_OK)
    {
        data->valid = false;
        return DHT11_ERR_NO_RESPONSE;
    }

    //========== 3. 接收 40 bit 数据 ==========
    for (uint8_t byte_idx = 0; byte_idx < 5; byte_idx++)
    {
        uint8_t byte_value = 0;

        for (uint8_t bit_idx = 0; bit_idx < 8; bit_idx++)
        {
            // 等待脉冲低电平（54μs）
            if (_Wait_Pin_Edge(0, 100) != DHT11_OK)
            {
                data->valid = false;
                return DHT11_ERR_TIMEOUT;
            }

            // 等待脉冲高电平
            if (_Wait_Pin_Edge(1, 100) != DHT11_OK)
            {
                data->valid = false;
                return DHT11_ERR_TIMEOUT;
            }

            // 测量高电平宽度，判断0或1
            pulse_width = _Measure_Pulse_Width(1);

            // 约 23μs 是0，68μs 是1
            // 判断阈值设为 40μs
            uint8_t bit_value = (pulse_width > 40) ? 1 : 0;

            // 将 bit 放入字节中
            byte_value = (byte_value << 1) | bit_value;
        }

        dht11_data[byte_idx] = byte_value;
    }

    //========== 4. 校验数据 ==========
    uint8_t checksum = dht11_data[0] + dht11_data[1] + dht11_data[2] + dht11_data[3];
    if (checksum != dht11_data[4])
    {
        data->valid = false;
        return DHT11_ERR_CHECKSUM;
    }

    //========== 5. 解析数据 ==========
    data->humidity = dht11_data[0];          // 湿度整数部分（DHT11没有小数部分）
    data->temperature = dht11_data[2];       // 温度整数部分
    data->valid = true;

    return DHT11_OK;
}

//========== 便捷接口 ==========
DHT11_Status_t DHT11_GetTemperature(float *temp)
{
    if (temp == NULL)
    {
        return DHT11_ERR_TIMEOUT;
    }

    DHT11_Data_t data;
    DHT11_Status_t status = DHT11_ReadData(&data);
    
    if (status == DHT11_OK && data.valid)
    {
        *temp = data.temperature;
        return DHT11_OK;
    }
    
    return status;
}

DHT11_Status_t DHT11_GetHumidity(float *humidity)
{
    if (humidity == NULL)
    {
        return DHT11_ERR_TIMEOUT;
    }

    DHT11_Data_t data;
    DHT11_Status_t status = DHT11_ReadData(&data);
    
    if (status == DHT11_OK && data.valid)
    {
        *humidity = data.humidity;
        return DHT11_OK;
    }
    
    return status;
}
