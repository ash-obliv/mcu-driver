#ifndef __ST7735S_H
#define __ST7735S_H

#include <stdint.h>
#include <stdbool.h>

//========== 色彩定义 ==========
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// 显示屏参数
#define TFT_WIDTH   128
#define TFT_HEIGHT  160

//========== 驱动函数指针结构 ==========
typedef struct {
    // 基础通信接口
    void (*gpio_init)(void);              // GPIO初始化
    void (*spi_write_byte)(uint8_t byte); // SPI写一字节
    void (*delay_ms)(uint32_t ms);        // 毫秒延时
    
    // 控制信号
    void (*set_dc_pin)(uint8_t level);    // DC引脚控制 (0=命令, 1=数据)
    void (*set_cs_pin)(uint8_t level);    // CS引脚控制 (0=选中, 1=释放)
    void (*set_rst_pin)(uint8_t level);   // RST引脚控制 (0=复位, 1=释放)
    void (*set_blk_pin)(uint8_t level);   // 背光控制 (0=关, 1=开)
} TFT_Driver_t;

//========== 驱动注册接口 ==========
void TFT_RegisterDriver(const TFT_Driver_t *driver);
bool TFT_IsDriverRegistered(void);

//========== 初始化接口 ==========
typedef enum {
    TFT_OK = 0,
    TFT_ERR_DRIVER_NOT_REGISTERED = -1,
    TFT_ERR_NULL_FUNC_PTR = -2,
    TFT_ERR_INIT_FAILED = -3,
} TFT_Status_t;

TFT_Status_t TFT_Init(void);
TFT_Status_t TFT_Uninit(void);

//========== 显示接口 ==========
void TFT_Clear(uint16_t color);
void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void TFT_DrawPoint(uint16_t x, uint16_t y, uint16_t color);

//========== 旋转和控制 ==========
void TFT_SetRotation(uint8_t rotation); // 0-3: 0°, 90°, 180°, 270°
void TFT_SetBacklight(uint8_t on);     // 0=关, 1=开

//========== 显示中文 ==========
void TFT_ShowChar(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, char c);
void TFT_ShowString(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const char *str);
void TFT_ShowNumber(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, long long num);
void TFT_ShowFloat(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, float num, uint8_t decimal_places);
int TFT_Printf(uint16_t x, uint16_t y, uint16_t color, const char *format, ...);

//========== 图形接口 ==========
void TFT_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint16_t width);
void TFT_DrawCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color, uint16_t width);
void TFT_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t width);

//========== 图像接口 ==========
void TFT_ShowImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *image);


#endif
