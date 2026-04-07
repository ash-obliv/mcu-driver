#include "middle_key.h"
#include "key.h"

// 这个要在使用的地方定义
extern Key_Handle_t g_key1;
extern Key_Handle_t g_key2;

KeyLevel_t gpio_readFunc(int pin_id)
{
    GPIO_PinState state = GPIO_PIN_SET; // 默认高电平
    // 按键读取函数
    if (pin_id == 1)
    {
        state = HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin);
    }
    else if (pin_id == 2)
    {
        state = HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin);
    }
    return (state == GPIO_PIN_RESET) ? KEY_LEVEL_LOW : KEY_LEVEL_HIGH;
}

uint32_t get_tickFunc(void)
{
    // 获取系统时间函数
    return HAL_GetTick();
}

void BSP_Key_Init(void)
{
    // 注册底层驱动接口
    Key_RegisterDriver(gpio_readFunc, get_tickFunc);

    Key_Config_t key1_config = {0};
    key1_config.pin_id = 1;                       // 按键1的ID为1
    key1_config.debounce_ms = 20;                 // 消抖时间 20
    key1_config.long_press_ms = 0;               // 长按判定时间 1000ms
    key1_config.active_level = KEY_LEVEL_LOW;    // 按键有效电平为低
    key1_config.double_click_ms = 0;            // 双击间隔窗口 300ms
    Key_ObjInit(&g_key1, &key1_config);

    Key_Config_t key2_config = {0};
    key2_config.pin_id = 2;                       // 按键2的ID为2
    key2_config.debounce_ms = 20;                // 消抖时间 15
    key2_config.long_press_ms = 1000;            // 长按判定时间 1000ms
    key2_config.active_level = KEY_LEVEL_LOW;    // 按键有效电平
    key2_config.double_click_ms = 0;             // 双击间隔窗口 300ms

    Key_ObjInit(&g_key2, &key2_config);
}
