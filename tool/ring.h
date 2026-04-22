// ring.h (Refactored: head/tail -> write_idx/read_idx)
#ifndef RING_H
#define RING_H

#include <stdint.h>

typedef struct
{
    uint8_t *buffer;
    uint32_t size;

    uint32_t write_idx; // 写指针
    uint32_t read_idx;  // 读指针

} ring_t;

void ring_init(ring_t *r, uint8_t *buf, uint32_t size);

uint32_t ring_available(const ring_t *r);
uint32_t ring_free(const ring_t *r);

uint32_t ring_write(ring_t *r, const uint8_t *data, uint32_t len);
uint32_t ring_read(ring_t *r, uint8_t *data, uint32_t len);

uint32_t ring_peek_linear(const ring_t *r, const uint8_t **ptr);
void ring_drop(ring_t *r, uint32_t len);

#endif
