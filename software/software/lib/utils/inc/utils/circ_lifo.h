/**
 * @file    circ_lifo.h
 * @brief   <Provide some short description here>
 */

#ifndef CIRC_LIFO_H_
#define CIRC_LIFO_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef CIRC_LIFO_FORMAT
#define CIRC_LIFO_FORMAT float
#endif

struct circ_lifo
{
	CIRC_LIFO_FORMAT* buffer;
	size_t mask;
	size_t i;
	bool dynamic;
};

int circ_lifo_construct(
	struct circ_lifo* const self,
	CIRC_LIFO_FORMAT* buffer,
	size_t capacity);
void circ_lifo_destruct(struct circ_lifo* const self);

void circ_lifo_write(struct circ_lifo* const self, CIRC_LIFO_FORMAT data);
CIRC_LIFO_FORMAT circ_lifo_read(struct circ_lifo* const self);

static inline CIRC_LIFO_FORMAT circ_lifo_peek(struct circ_lifo* const self, size_t index)
{
	return self->buffer[(self->i) + index & self->mask];
}

static inline size_t circ_lifo_capacity(struct circ_lifo* const self)
{
	return self->mask + 1u;
}

#endif /* RING_BUFFER_H_ */
