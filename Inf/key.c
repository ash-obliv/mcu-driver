#include "key.h"
#include <string.h>

/* ========================================================================== */
/*                           内部全局变量                                      */
/* ========================================================================== */

static Key_Driver_t s_key_driver = {0};
static Key_Driver_t *g_key_driver = &s_key_driver;
static bool g_driver_registered = false;

/* ========================================================================== */
/*                           内部辅助函数                                      */
/* ========================================================================== */

static inline bool Key_IsPressed_Raw(const Key_Handle_t *handle)
{
    KeyLevel_t level = g_key_driver->gpio_readFunc(handle->config.pin_id);
    return (level == handle->config.active_level);
}

/* ========================================================================== */
/*                           函数实现                                          */
/* ========================================================================== */

void Key_RegisterDriver(KeyLevel_t (*gpio_readFunc)(int pin_id),
                        uint32_t (*get_tickFunc)(void))
{
    if (gpio_readFunc == NULL || get_tickFunc == NULL)
        return;

    g_key_driver->gpio_readFunc = gpio_readFunc;
    g_key_driver->get_tickFunc = get_tickFunc;
    g_driver_registered = true;
}

void Key_ObjInit(Key_Handle_t *handle, const Key_Config_t *config)
{
    if (handle == NULL || config == NULL || !g_driver_registered)
        return;

    memset(handle, 0, sizeof(Key_Handle_t));
    handle->config = *config;
    handle->state = KEY_STATE_IDLE;
    handle->is_active = true;
}

int Key_ObjDeInit(Key_Handle_t *handle)
{
    if (handle == NULL)
        return -1;

    handle->state = KEY_STATE_IDLE;
    handle->is_active = false;
    handle->is_pressed = false;
    handle->press_start_time = 0;
    handle->release_start_time = 0;
    handle->press_duration = 0;
    handle->first_release_time = 0;
    return 0;
}

/**
 * @brief 核心扫描逻辑 (状态机)
 */
KeyEvent_t Key_Scan(Key_Handle_t *handle)
{
    if (handle == NULL || !g_driver_registered || !handle->is_active)
        return KEY_EVENT_NONE;

    KeyEvent_t event = KEY_EVENT_NONE;
    uint32_t current_time = g_key_driver->get_tickFunc();
    bool pressed_now = Key_IsPressed_Raw(handle);

    switch (handle->state)
    {
    // 当按键是空闲状态的时候 被按下 记录当前时间 进入按下消抖状态
    case KEY_STATE_IDLE:
        if (pressed_now)
        {
            handle->state = KEY_STATE_DEBOUNCING_PRESS;
            handle->press_start_time = current_time;
        }
        break;

    // 消抖状态 按键如果松开了 直接回到空闲状态 如果按着的时间超过了消抖时间 进入按下状态
    case KEY_STATE_DEBOUNCING_PRESS:
        if (!pressed_now)
        {
            handle->state = KEY_STATE_IDLE;
        }
        else if ((current_time - handle->press_start_time) >= handle->config.debounce_ms)
        {
            handle->state = KEY_STATE_PRESSED;
            handle->is_pressed = true;
        }
        break;

    // 当按键处于按下状态时 如果松开了 进入松开消抖状态 记录松开时间和按下持续时间
    case KEY_STATE_PRESSED:
        if (!pressed_now)
        {
            handle->state = KEY_STATE_DEBOUNCING_RELEASE;
            handle->release_start_time = current_time;
            handle->press_duration = current_time - handle->press_start_time;
        }
        break;

    // 现在按键是松开的状态 若松开的时候又被按下去了 说明是抖动 回到按下状态 继续等松开  超过消抖的时间还是无效电平说明是松开了 进入下一步判断
    case KEY_STATE_DEBOUNCING_RELEASE:
        if (pressed_now)
        {
            /* 抖动, 回去继续等松手 */
            handle->state = KEY_STATE_PRESSED;
        }
        else if ((current_time - handle->release_start_time) >= handle->config.debounce_ms)
        {
            handle->is_pressed = false;

            /* ---- 分支1: 写入了长按的时间逻辑 符合长按的逻辑  ---- */

            if (handle->config.long_press_ms > 0 && handle->press_duration >= handle->config.long_press_ms)
            {
                event = KEY_EVENT_LONG_PRESS;
                handle->state = KEY_STATE_IDLE;
            }
            /* ---- 分支2: 现在是短按的逻辑 但是开启了双击检测 ---- */
            else if (handle->config.double_click_ms > 0)
            {
                handle->state = KEY_STATE_WAIT_DOUBLE_CLICK;
                handle->first_release_time = current_time;
            }
            /* ---- 分支3: 非长按 + 未开启双击 → 直接短按 ---- */
            else
            {
                event = KEY_EVENT_SHORT_PRESS;
                handle->state = KEY_STATE_IDLE;
            }

            handle->press_duration = 0;
        }
        break;

        /* ================================================================== */
        /*  双击检测流程 (新增)                                                */
        /* ================================================================== */

    case KEY_STATE_WAIT_DOUBLE_CLICK:
        if (pressed_now)
        {
            /* 窗口内再次按下 → 进入第二次按下消抖 */
            handle->state = KEY_STATE_DEBOUNCING_DOUBLE;
            handle->press_start_time = current_time;
        }
        else if ((current_time - handle->first_release_time) >= handle->config.double_click_ms)
        {
            /* 超时没有第二次按下 → 判定为短按 */
            event = KEY_EVENT_SHORT_PRESS;
            handle->state = KEY_STATE_IDLE;
        }
        break;

    case KEY_STATE_DEBOUNCING_DOUBLE:
        if (!pressed_now)
        {
            /* 抖动, 回到等待窗口继续等 (窗口计时不重置) */
            handle->state = KEY_STATE_WAIT_DOUBLE_CLICK;
        }
        else if ((current_time - handle->press_start_time) >= handle->config.debounce_ms)
        {
            /* 第二次按下消抖通过 → 确认, 等松手 */
            handle->state = KEY_STATE_DOUBLE_PRESSED;
            handle->is_pressed = true;
        }
        break;

    case KEY_STATE_DOUBLE_PRESSED:
        if (!pressed_now)
        {
            /* 第二次松手 → 释放消抖 */
            handle->state = KEY_STATE_DEBOUNCING_DOUBLE_RELEASE;
            handle->release_start_time = current_time;
        }
        break;

    case KEY_STATE_DEBOUNCING_DOUBLE_RELEASE:
        if (pressed_now)
        {
            /* 抖动, 回去继续等松手 */
            handle->state = KEY_STATE_DOUBLE_PRESSED;
        }
        else if ((current_time - handle->release_start_time) >= handle->config.debounce_ms)
        {
            /* 第二次释放消抖通过 → 双击事件! */
            event = KEY_EVENT_DOUBLE_CLICK;
            handle->is_pressed = false;
            handle->state = KEY_STATE_IDLE;
        }
        break;

    default:
        handle->state = KEY_STATE_IDLE;
        handle->is_pressed = false;
        break;
    }

    return event;
}
