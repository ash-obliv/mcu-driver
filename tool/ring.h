#ifndef RING_H
#define RING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/*
 * 环形缓冲区（适用于串口 / DMA）：
 *
 * 规则：
 * - 空：head == tail
 * - 满：(tail + 1) % size == head
 * - 最大容量：size - 1
 *
 * 典型用法：
 *   - 中断 / DMA：写（producer）
 *   - 主循环     ：读（consumer）
 */

typedef struct {
    volatile uint32_t head; // 读指针（主循环）
    volatile uint32_t tail; // 写指针（中断）
    uint32_t size;          // buffer长度
    uint8_t *buf;           // 数据区
} ring_t;

enum {
    RING_OK = 0,
    RING_ERR_PARAM = -1,
    RING_ERR_RANGE = -2
};

int      ring_init(ring_t *r, uint8_t *buffer, uint32_t size);
void     ring_reset(ring_t *r);

uint32_t ring_available(const ring_t *r);  // 当前可读数据
uint32_t ring_free(const ring_t *r);       // 剩余可写空间

int      ring_is_empty(const ring_t *r);
int      ring_is_full(const ring_t *r);

/* 拷贝方式 */
uint32_t ring_write(ring_t *r, const uint8_t *data, uint32_t len);
uint32_t ring_read (ring_t *r, uint8_t *out, uint32_t len);

/* 零拷贝 */
uint32_t ring_peek_linear(const ring_t *r, const uint8_t **ptr);
int      ring_drop(ring_t *r, uint32_t n);

#ifdef __cplusplus
}
#endif

#endif