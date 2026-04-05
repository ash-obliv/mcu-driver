#ifndef __DHT11_H
#define __DHT11_H

#include <stdint.h>
#include <stdbool.h>

//========== DHT11 数据结构 ==========
typedef struct
{
    float humidity;    // 湿度 (0-100%)
    float temperature; // 温度 (-40~125°C)
    bool valid;        // 数据是否有效
} DHT11_Data_t;

//========== DHT11 返回值==========
typedef enum
{
    DHT11_OK = 0,
    DHT11_ERR_DRIVER_NOT_REGISTERED = -1,
    DHT11_ERR_NULL_FUNC_PTR = -2,
    DHT11_ERR_TIMEOUT = -3,
    DHT11_ERR_CHECKSUM = -4,
    DHT11_ERR_NO_RESPONSE = -5,
    DHT11_ERR_INIT_FAILED = -6,
} DHT11_Status_t;

//========== 驱动函数指针结构 ==========
typedef struct
{
    // GPIO 初始化
    void (*gpio_init)(void);

    // 引脚模式设置
    void (*pin_set_output)(void); // 设置为推挽输出（输出低电平需要主动驱动）
    void (*pin_set_input)(void);  // 设置为输入（释放引脚，让外部上拉器件拉高）

    // 引脚电平操作
    void (*pin_write)(uint8_t level); // 写引脚（0 或 1）
    uint8_t (*pin_read)(void);        // 读引脚状态

    // 延时
    void (*delay_us)(uint32_t us); // 微秒延时
} DHT11_Driver_t;

//========== 驱动注册接口 ==========
void DHT11_RegisterDriver(const DHT11_Driver_t *driver);
bool DHT11_IsDriverRegistered(void);

//========== 初始化接口 ==========
DHT11_Status_t DHT11_Init(void);
DHT11_Status_t DHT11_Uninit(void);

//========== 数据读取接口 ==========
DHT11_Status_t DHT11_ReadData(DHT11_Data_t *data);

//========== 便捷接口 ==========
DHT11_Status_t DHT11_GetTemperature(float *temp);
DHT11_Status_t DHT11_GetHumidity(float *humidity);

#endif /* __DHT11_H */
