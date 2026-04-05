/**
 * ============================================================
 * DHT11 框架集成示例
 * ============================================================
 * 
 * 这个文件展示如何在实际项目中使用 DHT11 抽象驱动框架。
 * 
 * 框架特点：
 * 1. 硬件和协议完全分离
 * 2. 单引脚动态模式切换（输出→输入）
 * 3. 支持快速迁移到其他MCU
 * 
 * 文件结构：
 * - Inf/dht11.c      : 通用 DHT11 协议驱动（与硬件无关）
 * - Inf/dht11.h      : 驱动接口定义
 * - Middle/middle_dht11.c : 硬件适配层（STM32 特定）
 * - Middle/middle_dht11.h : 适配层接口
 * 
 * ============================================================
 */

#include "main.h"
#include "spi.h"
#include "gpio.h"

/* 显示屏驱动 */
#include "st7735s.h"
#include "middle_tft.h"

/* DHT11 驱动 */
#include "dht11.h"
#include "middle_dht11.h"

/* USER CODE BEGIN PVD */

/* USER CODE END PVD */

/**
 * ============================================================
 * 示例 1：最简单的使用方法 - 定时读取并显示
 * ============================================================
 */
void Example_1_Simple_ReadAndDisplay(void)
{
    /* 初始化 */
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    
    /* 初始化屏幕 */
    BSP_TFT_Init();
    TFT_Clear(BLACK);
    TFT_ShowString(0, 0, 0xFFFF, BLACK, "DHT11 Demo");
    
    /* 初始化 DHT11 */
    DHT11_Status_t dht11_status = BSP_DHT11_Init();
    if (dht11_status != DHT11_OK)
    {
        TFT_ShowString(0, 20, 0xF800, BLACK, "DHT11 Init Failed");
        Error_Handler();
    }
    
    /* 主循环 */
    while (1)
    {
        DHT11_Data_t dht11_data;
        
        /* 读取数据 */
        DHT11_Status_t status = BSP_DHT11_ReadData(&dht11_data);
        
        if (status == DHT11_OK && dht11_data.valid)
        {
            /* 显示温度 */
            TFT_Printf(0, 30, 0x07E0, "Temp: %.1f C", dht11_data.temperature);
            
            /* 显示湿度 */
            TFT_Printf(0, 45, 0x07E0, "Humi: %.1f %%", dht11_data.humidity);
        }
        else
        {
            /* 显示错误信息 */
            switch (status)
            {
                case DHT11_ERR_TIMEOUT:
                    TFT_ShowString(0, 30, 0xF800, BLACK, "Timeout");
                    break;
                case DHT11_ERR_CHECKSUM:
                    TFT_ShowString(0, 30, 0xF800, BLACK, "Checksum Error");
                    break;
                case DHT11_ERR_NO_RESPONSE:
                    TFT_ShowString(0, 30, 0xF800, BLACK, "No Response");
                    break;
                default:
                    TFT_ShowString(0, 30, 0xF800, BLACK, "Read Error");
                    break;
            }
        }
        
        /* DHT11 建议最少间隔 2 秒读一次 */
        HAL_Delay(2000);
    }
}

/**
 * ============================================================
 * 示例 2：错误处理和重试机制
 * ============================================================
 */
void Example_2_WithErrorHandling(void)
{
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    
    BSP_TFT_Init();
    BSP_DHT11_Init();
    
    uint32_t read_count = 0;
    uint32_t success_count = 0;
    uint32_t fail_count = 0;
    
    while (1)
    {
        DHT11_Data_t dht11_data;
        DHT11_Status_t status;
        uint8_t retry = 0;
        
        /* 读取失败会自动重试 3 次 */
        do
        {
            status = BSP_DHT11_ReadData(&dht11_data);
            retry++;
            
            if (status == DHT11_OK)
                break;
                
            HAL_Delay(100);  /* 重试间隔 */
        } while (retry < 3 && status != DHT11_OK);
        
        read_count++;
        
        if (status == DHT11_OK && dht11_data.valid)
        {
            success_count++;
            
            /* 显示数据 */
            TFT_Printf(0, 0, 0xFFFF, "Temp: %.1f C", dht11_data.temperature);
            TFT_Printf(0, 15, 0xFFFF, "Humi: %.1f %%", dht11_data.humidity);
        }
        else
        {
            fail_count++;
            TFT_Printf(0, 30, 0xF800, "Error (retry %d)", retry);
        }
        
        /* 显示统计信息 */
        TFT_Printf(0, 50, 0x07E0, "R:%d S:%d F:%d", read_count, success_count, fail_count);
        
        HAL_Delay(2000);
    }
}

/**
 * ============================================================
 * 示例 3：多设备支持（多个 DHT11）
 * ============================================================
 * 
 * 注意：DHT11 是单线通信，同一时刻只能读一个。
 * 多设备需要分时复用或使用多个独立引脚。
 * 这里演示使用多个独立引脚的方案。
 * 
 */

/* 省略了 DHT11 #2 的定义，需要创建额外的 middle_dht11_2.c */
void Example_3_MultiDevice(void)
{
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    
    BSP_TFT_Init();
    BSP_DHT11_Init();      /* DHT11 #1: PA2 */
    // BSP_DHT11_2_Init();  /* DHT11 #2: PA3 */
    
    while (1)
    {
        DHT11_Data_t dht11_1, dht11_2;
        
        /* 读取 DHT11 #1 */
        if (BSP_DHT11_ReadData(&dht11_1) == DHT11_OK)
        {
            TFT_Printf(0, 0, 0xFFFF, "[1] T:%.1fC H:%.1f%%", 
                      dht11_1.temperature, dht11_1.humidity);
        }
        
        /* 读取 DHT11 #2 */
        // if (BSP_DHT11_2_ReadData(&dht11_2) == DHT11_OK)
        // {
        //     TFT_Printf(0, 30, 0xFFFF, "[2] T:%.1fC H:%.1f%%", 
        //           dht11_2.temperature, dht11_2.humidity);
        // }
        
        HAL_Delay(2000);
    }
}

/**
 * ============================================================
 * 示例 4：数据缓冲和平均值
 * ============================================================
 * 
 * DHT11 精度有限（±5%），可通过多次采样求平均来改善。
 */
typedef struct
{
    float temp_buffer[10];
    float humi_buffer[10];
    uint8_t sample_count;
    uint8_t buffer_index;
} DHT11_Buffer_t;

DHT11_Buffer_t g_dht11_buffer = {
    .sample_count = 10,
    .buffer_index = 0,
};

void DHT11_AddSample(DHT11_Data_t *data)
{
    if (data && data->valid)
    {
        g_dht11_buffer.temp_buffer[g_dht11_buffer.buffer_index] = data->temperature;
        g_dht11_buffer.humi_buffer[g_dht11_buffer.buffer_index] = data->humidity;
        
        g_dht11_buffer.buffer_index++;
        if (g_dht11_buffer.buffer_index >= g_dht11_buffer.sample_count)
            g_dht11_buffer.buffer_index = 0;
    }
}

float DHT11_GetAvgTemperature(void)
{
    float sum = 0;
    for (uint8_t i = 0; i < g_dht11_buffer.sample_count; i++)
        sum += g_dht11_buffer.temp_buffer[i];
    return sum / g_dht11_buffer.sample_count;
}

float DHT11_GetAvgHumidity(void)
{
    float sum = 0;
    for (uint8_t i = 0; i < g_dht11_buffer.sample_count; i++)
        sum += g_dht11_buffer.humi_buffer[i];
    return sum / g_dht11_buffer.sample_count;
}

void Example_4_WithAverage(void)
{
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    
    BSP_TFT_Init();
    BSP_DHT11_Init();
    
    TFT_ShowString(0, 0, 0xFFFF, BLACK, "Averaging...");
    
    while (1)
    {
        DHT11_Data_t data;
        
        if (BSP_DHT11_ReadData(&data) == DHT11_OK)
        {
            DHT11_AddSample(&data);
            
            /* 显示实时值 */
            TFT_Printf(0, 20, 0x07E0, "Real: T:%.1fC H:%.1f%%", 
                      data.temperature, data.humidity);
            
            /* 显示平均值 */
            TFT_Printf(0, 40, 0xFFFF, "Avg:  T:%.1fC H:%.1f%%", 
                      DHT11_GetAvgTemperature(), DHT11_GetAvgHumidity());
        }
        
        HAL_Delay(2000);
    }
}

/**
 * ============================================================
 * 主函数：选择要运行的示例
 * ============================================================
 */
int main(void)
{
    /* HAL库初始化 */
    HAL_Init();
    
    /* 选择要运行的示例 */
    // Example_1_Simple_ReadAndDisplay();      /* 最简单 */
    Example_2_WithErrorHandling();            /* 推荐：带错误处理 */
    // Example_3_MultiDevice();                /* 多设备 */
    // Example_4_WithAverage();                /* 数据平均 */
    
    /* 不会执行到这里 */
    return 0;
}

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
