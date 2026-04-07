#ifndef __KEY_H__
#define __KEY_H__

#include <stdint.h>
#include <stdbool.h>

/* ========================================================================== */
/*                           类型定义 (Type Definitions)                      */
/* ========================================================================== */

typedef enum
{
    KEY_LEVEL_LOW = 0,
    KEY_LEVEL_HIGH = 1,
} KeyLevel_t;

typedef enum
{
    KEY_EVENT_NONE = 0,
    KEY_EVENT_SHORT_PRESS,     /**< 短按 (松手后触发) */
    KEY_EVENT_LONG_PRESS,      /**< 长按 (松手后触发) */
    KEY_EVENT_DOUBLE_CLICK,    /**< 双击 (第二次松手后触发) */
} KeyEvent_t;

typedef enum
{
    KEY_STATE_IDLE = 0,             /* 空闲状态, 等待按下 */
    KEY_STATE_DEBOUNCING_PRESS,     /* 按下消抖中, 等待稳定按下 */
    KEY_STATE_PRESSED,              /* 按下已确认, 等待松手 */
    KEY_STATE_DEBOUNCING_RELEASE,   /* 松开消抖中, 等待稳定松开 */
    KEY_STATE_WAIT_DOUBLE_CLICK,    /**< 第一次松手后, 等待可能的第二次点击 */
    KEY_STATE_DEBOUNCING_DOUBLE,    /**< 第二次按下消抖中 */
    KEY_STATE_DOUBLE_PRESSED,       /**< 第二次按下已确认, 等待松手 */
    KEY_STATE_DEBOUNCING_DOUBLE_RELEASE, /**< 第二次松手消抖中 */
} KeyState_t;

typedef struct
{
    int        pin_id;
    KeyLevel_t active_level;
    uint32_t   debounce_ms;       /**< 消抖时间 (ms), 建议 10~20 */
    uint32_t   long_press_ms;     /**< 长按阈值 (ms), 0=不检测长按 */
    uint32_t   double_click_ms;   /**< 双击间隔窗口 (ms), 0=不检测双击, 建议 200~400 */
} Key_Config_t;

typedef struct
{
    Key_Config_t config;

    KeyState_t state;

    bool     is_active;
    bool     is_pressed;
    uint32_t press_start_time;
    uint32_t release_start_time;
    uint32_t press_duration;
    uint32_t first_release_time;    /**< 第一次松手时刻 (用于双击窗口计时) */
} Key_Handle_t;

typedef struct
{
    KeyLevel_t (*gpio_readFunc)(int pin_id);
    uint32_t   (*get_tickFunc)(void);
} Key_Driver_t;

/* ========================================================================== */
/*                           API                                              */
/* ========================================================================== */

void       Key_RegisterDriver(KeyLevel_t (*gpio_readFunc)(int pin_id),
                           uint32_t (*get_tickFunc)(void));
void       Key_ObjInit(Key_Handle_t *handle, const Key_Config_t *config);
int        Key_ObjDeInit(Key_Handle_t *handle);
KeyEvent_t Key_Scan(Key_Handle_t *handle);

#endif /* __KEY_H__ */
