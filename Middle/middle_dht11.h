#ifndef __MIDDLE_DHT11_H
#define __MIDDLE_DHT11_H

#include "dht11.h"

//========== DHT11 硬件引脚定义 ==========
// 根据你的硬件修改这些定义
// 示例：
// #define DHT11_DATA_GPIO_PORT    GPIOC
// #define DHT11_DATA_GPIO_PIN     GPIO_PIN_0

//========== 初始化接口 ==========
/**
 * BSP_DHT11_Init - 初始化DHT11模块
 * 
 * 这个函数会：
 * 1. 初始化GPIO
 * 2. 注册驱动函数指针
 * 3. 调用DHT11_Init()
 * 
 * 返回值：DHT11_OK 或对应的错误码
 */
DHT11_Status_t BSP_DHT11_Init(void);

//========== 便捷读取接口 ==========
DHT11_Status_t BSP_DHT11_ReadData(DHT11_Data_t *data);
DHT11_Status_t BSP_DHT11_GetTemperature(float *temp);
DHT11_Status_t BSP_DHT11_GetHumidity(float *humidity);

#endif /* __MIDDLE_DHT11_H */
