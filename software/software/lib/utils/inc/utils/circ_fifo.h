/**
 * @file    circ_fifo.h
 * @brief   <Provide some short description here>
 */

#ifndef CIRC_FIFO_H_
#define CIRC_FIFO_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct circ_fifo
{
        float* buffer;
        size_t mask;
        size_t i;
        bool dynamic;
};

#define circ_fifo_peek(cf, off) (cf->buffer[(cf->i + (off)) & cf->mask])

int circ_fifo_construct(
        struct circ_fifo* const self, float* const buffer, size_t capacity);
void circ_fifo_destruct(struct circ_fifo* const self);

void circ_fifo_write(struct circ_fifo* const self, float data);
float circ_fifo_read(struct circ_fifo* const self);

float* circ_fifo_purge(struct circ_fifo* const self);

static inline size_t circ_fifo_capacity(struct circ_fifo* const self)
{
        return self->mask + 1u;
}

#endif /* RING_BUFFER_H_ */
