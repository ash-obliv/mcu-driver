// ring.c
#include "ring.h"

static uint32_t min_u32(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

void ring_init(ring_t *r, uint8_t *buf, uint32_t size)
{
    r->buffer = buf;
    r->size = size;
    r->write_idx = 0;
    r->read_idx = 0;
}

uint32_t ring_available(const ring_t *r)
{
    if (r->write_idx >= r->read_idx)
        return r->write_idx - r->read_idx;
    else
        return r->size - (r->read_idx - r->write_idx);
}

uint32_t ring_free(const ring_t *r)
{
    return r->size - ring_available(r);
}

uint32_t ring_write(ring_t *r, const uint8_t *data, uint32_t len)
{
    uint32_t free = ring_free(r);
    len = min_u32(len, free);

    uint32_t first = min_u32(len, r->size - r->write_idx);

    for (uint32_t i = 0; i < first; i++)
        r->buffer[r->write_idx + i] = data[i];

    for (uint32_t i = 0; i < len - first; i++)
        r->buffer[i] = data[first + i];

    r->write_idx = (r->write_idx + len) % r->size;

    return len;
}

uint32_t ring_read(ring_t *r, uint8_t *data, uint32_t len)
{
    uint32_t avail = ring_available(r);
    len = min_u32(len, avail);

    uint32_t first = min_u32(len, r->size - r->read_idx);

    for (uint32_t i = 0; i < first; i++)
        data[i] = r->buffer[r->read_idx + i];

    for (uint32_t i = 0; i < len - first; i++)
        data[first + i] = r->buffer[i];

    r->read_idx = (r->read_idx + len) % r->size;

    return len;
}

uint32_t ring_peek_linear(const ring_t *r, const uint8_t **ptr)
{
    uint32_t avail = ring_available(r);
    if (avail == 0)
    {
        *ptr = 0;
        return 0;
    }

    *ptr = &r->buffer[r->read_idx];

    if (r->write_idx > r->read_idx)
        return r->write_idx - r->read_idx;
    else
        return r->size - r->read_idx;
}

void ring_drop(ring_t *r, uint32_t len)
{
    uint32_t avail = ring_available(r);
    if (len > avail)
        len = avail;

    r->read_idx = (r->read_idx + len) % r->size;
}
