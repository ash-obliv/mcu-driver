#include "st7735s.h"
#include <string.h>
#include <math.h>
#include "st7735s_data.h"
#include <stdarg.h>
#include <stdio.h>

//========== 全局驱动指针 ==========
static const TFT_Driver_t *g_tft_driver = NULL;
static bool g_tft_initialized = false;

// 显示屏配置（可根据不同屏幕调整）
typedef struct
{
    uint8_t rotation; // 当前旋转角度 0-3
    uint16_t width;   // 当前宽度
    uint16_t height;  // 当前高度
} TFT_Config_t;

static TFT_Config_t g_tft_config = {
    .rotation = 0,
    .width = TFT_WIDTH,
    .height = TFT_HEIGHT,
};

//========== 驱动管理 ==========
void TFT_RegisterDriver(const TFT_Driver_t *driver)
{
    if (driver == NULL)
    {
        return;
    }
    g_tft_driver = driver;
}

bool TFT_IsDriverRegistered(void)
{
    return g_tft_driver != NULL;
}

//========== 驱动函数验证 ==========
static TFT_Status_t TFT_VerifyDriver(void)
{
    if (g_tft_driver == NULL)
    {
        return TFT_ERR_DRIVER_NOT_REGISTERED;
    }

    // 检查所有关键函数指针
    if (g_tft_driver->gpio_init == NULL ||
        g_tft_driver->spi_write_byte == NULL ||
        g_tft_driver->delay_ms == NULL ||
        g_tft_driver->set_dc_pin == NULL ||
        g_tft_driver->set_cs_pin == NULL ||
        g_tft_driver->set_rst_pin == NULL ||
        g_tft_driver->set_blk_pin == NULL)
    {
        return TFT_ERR_NULL_FUNC_PTR;
    }

    return TFT_OK;
}

//========== 基础通信函数 ==========
static void _TFT_SendByte(uint8_t byte, uint8_t is_data)
{
    if (g_tft_driver == NULL)
        return;

    // DC控制：0=命令, 1=数据
    g_tft_driver->set_dc_pin(is_data);

    // CS拉低，片选
    g_tft_driver->set_cs_pin(0);

    // 发送字节
    g_tft_driver->spi_write_byte(byte);

    // CS拉高，释放片选
    g_tft_driver->set_cs_pin(1);
}

static void _TFT_SendCmd(uint8_t cmd)
{
    _TFT_SendByte(cmd, 0); // is_data = 0，表示发送命令
}

static void _TFT_SendData(uint8_t data)
{
    _TFT_SendByte(data, 1); // is_data = 1，表示发送数据
}

static void _TFT_SendData16(uint16_t data)
{
    _TFT_SendData(data >> 8);   // 高字节
    _TFT_SendData(data & 0xFF); // 低字节
}

// static void _TFT_SendCmdWithData(uint8_t cmd, const uint8_t *data, uint8_t len)
//{
//     _TFT_SendCmd(cmd);
//     for (uint8_t i = 0; i < len; i++)
//     {
//         _TFT_SendData(data[i]);
//     }
// }

//========== 屏幕寻址 ==========
static void _TFT_SetAddressWindow(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    // 设置列地址范围 (0x2A)
    _TFT_SendCmd(0x2A);
    _TFT_SendData(0x00);
    _TFT_SendData(x_start);
    _TFT_SendData(0x00);
    _TFT_SendData(x_end);

    // 设置行地址范围 (0x2B)
    _TFT_SendCmd(0x2B);
    _TFT_SendData(0x00);
    _TFT_SendData(y_start);
    _TFT_SendData(0x00);
    _TFT_SendData(y_end);

    // 准备写入内存 (0x2C)
    _TFT_SendCmd(0x2C);
}

//========== 初始化 ==========
TFT_Status_t TFT_Init(void)
{
    TFT_Status_t status;

    // 验证驱动是否注册
    status = TFT_VerifyDriver();
    if (status != TFT_OK)
    {
        return status;
    }

    // GPIO初始化
    g_tft_driver->gpio_init();

    // 硬件复位
    g_tft_driver->set_rst_pin(0);
    g_tft_driver->delay_ms(100);
    g_tft_driver->set_rst_pin(1);
    g_tft_driver->delay_ms(120);

    // ========== ST7735S初始化序列 ==========

    // 1. 睡眠模式关闭 (0x11)
    _TFT_SendCmd(0x11);
    g_tft_driver->delay_ms(120);

    // 2. 内存数据访问控制 (0x36)
    _TFT_SendCmd(0x36);
    _TFT_SendData(0xC0); // MY=1, MX=1, RGB模式

    // 3. 设置像素显示格式 表示这个显示屏使用16位RGB565格式，每个像素占用2字节数据。0x05表示16位颜色模式。 (0x3A)
    _TFT_SendCmd(0x3A);
    _TFT_SendData(0x05); // 16位 RGB565

    // 4. 帧率控制 - 全色模式 (0xB1)
    _TFT_SendCmd(0xB1);
    _TFT_SendData(0x05);
    _TFT_SendData(0x3C);
    _TFT_SendData(0x3C);

    // 5. 帧率控制 - 空闲模式 (0xB2)
    _TFT_SendCmd(0xB2);
    _TFT_SendData(0x05);
    _TFT_SendData(0x3C);
    _TFT_SendData(0x3C);

    // 6. 帧率控制 - 部分模式 (0xB3)
    _TFT_SendCmd(0xB3);
    _TFT_SendData(0x05);
    _TFT_SendData(0x3C);
    _TFT_SendData(0x3C);
    _TFT_SendData(0x05);
    _TFT_SendData(0x3C);
    _TFT_SendData(0x3C);

    // 7. 显示反转控制 (0xB4)
    _TFT_SendCmd(0xB4);
    _TFT_SendData(0x03);

    // 8. 电源控制 (0xC0-0xC5)
    _TFT_SendCmd(0xC0);
    _TFT_SendData(0x2E);
    _TFT_SendData(0x06);
    _TFT_SendData(0x04);

    _TFT_SendCmd(0xC1);
    _TFT_SendData(0xC0);
    _TFT_SendData(0xC2);

    _TFT_SendCmd(0xC2);
    _TFT_SendData(0x0D);
    _TFT_SendData(0x0D);

    _TFT_SendCmd(0xC3);
    _TFT_SendData(0x8D);
    _TFT_SendData(0xEE);

    _TFT_SendCmd(0xC4);
    _TFT_SendData(0x8D);
    _TFT_SendData(0xEE);

    _TFT_SendCmd(0xC5);
    _TFT_SendData(0x00);

    // 9. 伽马校正曲线 (0xE0)
    _TFT_SendCmd(0xE0);
    _TFT_SendData(0x1B);
    _TFT_SendData(0x21);
    _TFT_SendData(0x10);
    _TFT_SendData(0x15);
    _TFT_SendData(0x2B);
    _TFT_SendData(0x25);
    _TFT_SendData(0x1F);
    _TFT_SendData(0x23);
    _TFT_SendData(0x22);
    _TFT_SendData(0x22);
    _TFT_SendData(0x2B);
    _TFT_SendData(0x37);
    _TFT_SendData(0x00);
    _TFT_SendData(0x15);
    _TFT_SendData(0x02);
    _TFT_SendData(0x3F);

    // 10. 伽马校正曲线 (0xE1)
    _TFT_SendCmd(0xE1);
    _TFT_SendData(0x1A);
    _TFT_SendData(0x20);
    _TFT_SendData(0x0F);
    _TFT_SendData(0x15);
    _TFT_SendData(0x2A);
    _TFT_SendData(0x25);
    _TFT_SendData(0x1E);
    _TFT_SendData(0x23);
    _TFT_SendData(0x23);
    _TFT_SendData(0x22);
    _TFT_SendData(0x2B);
    _TFT_SendData(0x37);
    _TFT_SendData(0x00);
    _TFT_SendData(0x15);
    _TFT_SendData(0x02);
    _TFT_SendData(0x3F);

    // 11. 显示开启 (0x29)
    _TFT_SendCmd(0x29);

    // 12. 背光开启
    g_tft_driver->set_blk_pin(1);

    g_tft_initialized = true;
    return TFT_OK;
}

TFT_Status_t TFT_Uninit(void)
{
    if (!g_tft_initialized)
    {
        return TFT_OK;
    }

    g_tft_driver->set_blk_pin(0); // 关闭背光
    _TFT_SendCmd(0x28);           // 显示关闭
    g_tft_driver->delay_ms(50);
    _TFT_SendCmd(0x10); // 进入睡眠模式

    g_tft_initialized = false;
    return TFT_OK;
}

//========== 显示接口 ==========
void TFT_Clear(uint16_t color)
{
    if (!g_tft_initialized)
        return;

    TFT_FillRect(0, 0, g_tft_config.width - 1, g_tft_config.height - 1, color);
}

void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (!g_tft_initialized)
        return;

    _TFT_SetAddressWindow(x, y, x + w, y + h);

    uint32_t pixel_count = (w + 1) * (h + 1);
    for (uint32_t i = 0; i < pixel_count; i++)
    {
        _TFT_SendData16(color);
    }
}

void TFT_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    if (!g_tft_initialized)
        return;

    _TFT_SetAddressWindow(x, y, x, y);
    _TFT_SendData16(color);
}

//========== 旋转 ==========
void TFT_SetRotation(uint8_t rotation)
{
    if (!g_tft_initialized || rotation > 3)
        return;

    g_tft_config.rotation = rotation;

    _TFT_SendCmd(0x36);

    switch (rotation)
    {
    case 0: // 0°
        _TFT_SendData(0xC0);
        g_tft_config.width = TFT_WIDTH;
        g_tft_config.height = TFT_HEIGHT;
        break;
    case 1: // 90°
        _TFT_SendData(0xA0);
        g_tft_config.width = TFT_HEIGHT;
        g_tft_config.height = TFT_WIDTH;
        break;
    case 2: // 180°
        _TFT_SendData(0x00);
        g_tft_config.width = TFT_WIDTH;
        g_tft_config.height = TFT_HEIGHT;
        break;
    case 3: // 270°
        _TFT_SendData(0x60);
        g_tft_config.width = TFT_HEIGHT;
        g_tft_config.height = TFT_WIDTH;
        break;
    default:
        break;
    }
}

void TFT_SetBacklight(uint8_t on)
{
    if (g_tft_driver != NULL)
    {
        g_tft_driver->set_blk_pin(on ? 1 : 0);
    }
}

//========== 显示文本 ==========
void TFT_ShowChar(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, char c)
{
    if (!g_tft_initialized)
        return;

    // 检查字符范围 (ASCII 32-126)
    if (c < 32 || c > 126)
        return;

    // 获取字体数据索引
    uint8_t index = c - 32;
    const uint8_t *font_data = g_font_5x8_ascii[index];

    // 设置显示窗口 (5×8像素)
    _TFT_SetAddressWindow(x, y, x + 4, y + 7);

    // 逐字节绘制字体
    for (uint8_t row = 0; row < 8; row++)
    {
        uint8_t byte = font_data[row];
        for (uint8_t col = 0; col < 5; col++)
        {
            // 从MSB到LSB逐位检查
            if (byte & (0x80 >> col))
            {
                _TFT_SendData16(fc); // 前景色
            }
            else
            {
                _TFT_SendData16(bc); // 背景色
            }
        }
    }
}

//========== 显示字符串 ==========
void TFT_ShowString(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const char *str)
{
    if (!g_tft_initialized || str == NULL)
        return;

    uint16_t curr_x = x;
    uint16_t curr_y = y;
    uint8_t char_width = 5;   // 字符宽度
    uint8_t char_height = 8;  // 字符高度
    uint8_t char_spacing = 1; // 字符间距

    while (*str)
    {
        // 换行处理
        if (curr_x + char_width >= g_tft_config.width)
        {
            curr_x = x;
            curr_y += char_height + char_spacing;
        }

        // 超出屏幕高度
        if (curr_y + char_height >= g_tft_config.height)
        {
            break;
        }

        TFT_ShowChar(curr_x, curr_y, fc, bc, *str);
        curr_x += char_width + char_spacing;
        str++;
    }
}

//========== 显示数字 ==========
void TFT_ShowNumber(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, long long num)
{
    if (!g_tft_initialized)
        return;

    // 处理负数
    char buffer[32] = {0};
    if (num < 0)
    {
        buffer[0] = '-';
        num = -num;
    }

    // 转换数字为字符串
    uint8_t index = (num < 0) ? 1 : 0;
    if (num == 0)
    {
        buffer[index] = '0';
        buffer[index + 1] = '\0';
    }
    else
    {
        // 反向填充数字
        uint8_t temp_index = index;
        long long temp_num = num;
        while (temp_num > 0)
        {
            temp_index++;
            temp_num /= 10;
        }
        buffer[temp_index] = '\0';

        while (num > 0)
        {
            buffer[--temp_index] = '0' + (num % 10);
            num /= 10;
        }
    }

    TFT_ShowString(x, y, fc, bc, buffer);
}

//========== 显示浮点数 ==========
void TFT_ShowFloat(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, float num, uint8_t decimal_places)
{
    if (!g_tft_initialized)
        return;

    char buffer[32] = {0};

    // 简单的浮点数转字符串
    if (num < 0)
    {
        buffer[0] = '-';
        num = -num;
    }

    long long int_part = (long long)num;
    long long frac_part = (long long)((num - int_part) * pow(10, decimal_places));

    // 格式化字符串
    sprintf(buffer, "%lld.%0*lld", int_part, decimal_places, frac_part);

    TFT_ShowString(x, y, fc, bc, buffer);
}

//========== 简化版Printf - 一个函数搞定 ==========
int TFT_Printf(uint16_t x, uint16_t y, uint16_t color, const char *format, ...)
{
    if (!g_tft_initialized || format == NULL)
        return -1;

    char buffer[256]; // 缓冲区
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len <= 0)
        return len;

    // 逐字符显示
    uint16_t curr_x = x;
    uint16_t curr_y = y;
    uint8_t char_w = 5;  // 字符宽度
    uint8_t char_h = 8;  // 字符高度
    uint8_t spacing = 1; // 间距

    for (int i = 0; i < len; i++)
    {
        char c = buffer[i];

        // 处理特殊字符
        if (c == '\n')
        {
            curr_x = x; // 回到起始X
            curr_y += char_h + spacing;
            continue;
        }
        else if (c == '\r')
        {
            curr_x = x;
            continue;
        }
        else if (c == '\t')
        {
            curr_x += (char_w + spacing) * 4;
            continue;
        }

        // 自动换行
        if (curr_x + char_w >= g_tft_config.width)
        {
            curr_x = x;
            curr_y += char_h + spacing;
        }

        // 超出屏幕高度则停止
        if (curr_y + char_h >= g_tft_config.height)
            break;

        // 显示字符
        TFT_ShowChar(curr_x, curr_y, color, 0x0000, c);
        curr_x += char_w + spacing;
    }

    return len;
}

//========== 绘制线条 (Bresenham算法 + 线宽) ==========
void TFT_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint16_t width)
{
    if (!g_tft_initialized || width == 0)
        return;

    int16_t dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int16_t dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    int16_t x = x0;
    int16_t y = y0;

    while (1)
    {
        // 绘制当前像素 (考虑线宽)
        if (width == 1)
        {
            TFT_DrawPoint(x, y, color);
        }
        else
        {
            // 对于较粗的线条，使用矩形填充
            int16_t half_width = width / 2;
            TFT_FillRect(x - half_width, y - half_width, width - 1, width - 1, color);
        }

        // 检查是否到达终点
        if (x == x1 && y == y1)
            break;

        // Bresenham决策
        int16_t e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y += sy;
        }
    }
}

//========== 绘制圆形 (中点圆算法 + 线宽) ==========
void TFT_DrawCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color, uint16_t width)
{
    if (!g_tft_initialized || width == 0 || radius == 0)
        return;

    int16_t r = radius;
    int16_t px = 0;
    int16_t py = r;
    int16_t d = 3 - 2 * r; // 决策参数

    while (px <= py)
    {
        // 绘制8个对称点
        if (width == 1)
        {
            TFT_DrawPoint(x + px, y + py, color);
            TFT_DrawPoint(x - px, y + py, color);
            TFT_DrawPoint(x + px, y - py, color);
            TFT_DrawPoint(x - px, y - py, color);
            TFT_DrawPoint(x + py, y + px, color);
            TFT_DrawPoint(x - py, y + px, color);
            TFT_DrawPoint(x + py, y - px, color);
            TFT_DrawPoint(x - py, y - px, color);
        }
        else
        {
            // 对于较粗的圆，使用小矩形填充
            uint16_t hw = (width + 1) / 2; // 半线宽
            TFT_FillRect(x + px - hw, y + py - hw, width - 1, width - 1, color);
            TFT_FillRect(x - px - hw, y + py - hw, width - 1, width - 1, color);
            TFT_FillRect(x + px - hw, y - py - hw, width - 1, width - 1, color);
            TFT_FillRect(x - px - hw, y - py - hw, width - 1, width - 1, color);
            TFT_FillRect(x + py - hw, y + px - hw, width - 1, width - 1, color);
            TFT_FillRect(x - py - hw, y + px - hw, width - 1, width - 1, color);
            TFT_FillRect(x + py - hw, y - px - hw, width - 1, width - 1, color);
            TFT_FillRect(x - py - hw, y - px - hw, width - 1, width - 1, color);
        }

        // 中点圆决策
        if (d < 0)
        {
            d = d + 4 * px + 6;
        }
        else
        {
            d = d + 4 * (px - py) + 10;
            py--;
        }
        px++;
    }
}

//========== 绘制矩形 (使用线条+线宽) ==========
void TFT_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t width)
{
    if (!g_tft_initialized || width == 0)
        return;

    // 绘制四条边
    TFT_DrawLine(x, y, x + w, y, color, width);         // 上边
    TFT_DrawLine(x + w, y, x + w, y + h, color, width); // 右边
    TFT_DrawLine(x + w, y + h, x, y + h, color, width); // 下边
    TFT_DrawLine(x, y + h, x, y, color, width);         // 左边
}

//========== 图像 ==========
void TFT_ShowImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *image)
{
    if (!g_tft_initialized || image == NULL)
        return;

    _TFT_SetAddressWindow(x, y, x + width - 1, y + height - 1);

    for (uint32_t i = 0; i < width * height; i++)
    {
        uint8_t high = image[2 * i];
        uint8_t low = image[2 * i + 1];
        _TFT_SendData(high);
        _TFT_SendData(low);
    }
}
