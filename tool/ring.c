#include "ring.h"
#include <string.h>

/* 获取下一个索引（自动回绕） */
static inline uint32_t ring_next(const ring_t *r, uint32_t idx)
{
    idx++;
    if (idx >= r->size)
        idx = 0;
    return idx;
}

/* 初始化环形缓冲区 */
int ring_init(ring_t *r, uint8_t *buffer, uint32_t size)
{
    if (!r || !buffer)
        return RING_ERR_PARAM;

    if (size < 2)
        return RING_ERR_RANGE; // 至少要留一个空位

    r->buf = buffer;
    r->size = size;
    r->head = 0;
    r->tail = 0;

    return RING_OK;
}

/* 清空缓冲区 */
void ring_reset(ring_t *r)
{
    if (!r)
        return;

    r->head = 0;
    r->tail = 0;
}

/* 判断是否为空 */
int ring_is_empty(const ring_t *r)
{
    return (r && (r->head == r->tail));
}

/* 判断是否已满 */
int ring_is_full(const ring_t *r)
{
    if (!r)
        return 0;

    return (ring_next(r, r->tail) == r->head);
}

/* 当前可读数据长度 */
uint32_t ring_available(const ring_t *r)
{
    if (!r)
        return 0;

    uint32_t head = r->head;
    uint32_t tail = r->tail;

    if (tail >= head)
        return tail - head;
    else
        return r->size - (head - tail);
}

/* 剩余可写空间 */
uint32_t ring_free(const ring_t *r)
{
    if (!r)
        return 0;

    return (r->size - 1u) - ring_available(r);
}

/* 写数据（适用于串口中断） */
uint32_t ring_write(ring_t *r, const uint8_t *data, uint32_t len)
{
    if (!r || !data || len == 0)
        return 0;

    uint32_t free = ring_free(r);
    if (free == 0)
        return 0;

    /* 实际写入长度不能超过剩余空间 */
    if (len > free)
        len = free;

    uint32_t tail = r->tail;

    /* 第一段：从 tail 写到 buffer 末尾 */
    uint32_t first_part = r->size - tail;
    if (first_part > len)
        first_part = len;

    memcpy(&r->buf[tail], data, first_part);

    /* 第二段：从 buffer 开头写 */
    uint32_t second_part = len - first_part;
    if (second_part > 0)
    {
        memcpy(&r->buf[0], data + first_part, second_part);
    }

    /* 更新 tail（回绕） */
    r->tail = (tail + len) % r->size;

    return len;
}

/* 读数据（主循环使用） */
uint32_t ring_read(ring_t *r, uint8_t *out, uint32_t len)
{
    if (!r || !out || len == 0)
        return 0;

    uint32_t avail = ring_available(r);
    if (avail == 0)
        return 0;

    /* 实际读取长度 */
    if (len > avail)
        len = avail;

    uint32_t head = r->head;

    /* 第一段：从 head 到 buffer 末尾 */
    uint32_t first_part = r->size - head;
    if (first_part > len)
        first_part = len;

    memcpy(out, &r->buf[head], first_part);

    /* 第二段：从 buffer 开头 */
    uint32_t second_part = len - first_part;
    if (second_part > 0)
    {
        memcpy(out + first_part, &r->buf[0], second_part);
    }

    /* 更新 head */
    r->head = (head + len) % r->size;

    return len;
}

/* 获取一段连续可读内存（零拷贝） */
uint32_t ring_peek_linear(const ring_t *r, const uint8_t **ptr)
{
    if (!r || !ptr)
        return 0;

    uint32_t avail = ring_available(r);
    if (avail == 0)
    {
        *ptr = NULL;
        return 0;
    }

    *ptr = &r->buf[r->head];

    uint32_t to_end = r->size - r->head;

    return (avail < to_end) ? avail : to_end;
}

/* 丢弃数据（配合 peek 使用） */
int ring_drop(ring_t *r, uint32_t n)
{
    if (!r)
        return RING_ERR_PARAM;

    uint32_t avail = ring_available(r);
    if (n > avail)
        return RING_ERR_RANGE;

    r->head = (r->head + n) % r->size;

    return RING_OK;
}