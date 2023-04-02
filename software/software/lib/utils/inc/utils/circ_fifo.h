/**
 * @file    circ_fifo.h
 * @brief   <Provide some short description here>
 */

#ifndef CIRC_FIFO_H_
#define CIRC_FIFO_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef CIRC_FIFO_FORMAT
#define CIRC_FIFO_FORMAT float
#endif

struct circ_fifo
{
        CIRC_FIFO_FORMAT* buffer;
        size_t mask;
        size_t i;
        bool dynamic;
};

int circ_fifo_construct(
        struct circ_fifo* const self,
        CIRC_FIFO_FORMAT* buffer,
        size_t capacity);
void circ_fifo_destruct(struct circ_fifo* const self);

void circ_fifo_write(struct circ_fifo* const self, CIRC_FIFO_FORMAT data);
CIRC_FIFO_FORMAT circ_fifo_read(struct circ_fifo* const self);

CIRC_FIFO_FORMAT* circ_fifo_purge(struct circ_fifo* const self);

static inline CIRC_FIFO_FORMAT circ_fifo_peek(struct circ_fifo* const self, size_t index)
{
        return self->buffer[(self->i) + index & self->mask];
}

static inline size_t circ_fifo_capacity(struct circ_fifo* const self)
{
        return self->mask + 1u;
}

#endif /* RING_BUFFER_H_ */
