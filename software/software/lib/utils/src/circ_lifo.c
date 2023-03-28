/**
 * @file    circ_lifo.c
 * @brief   <Provide some short description here>
 */

#include "utils/circ_lifo.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "logger/logger.h"
#include "utils/assert.h"

static inline void circ_lifo_deallocate(struct circ_lifo* const self)
{
	ASSERT(self != NULL);

	if (self->dynamic)
		free(self->buffer);
}

static inline bool circ_lifo_is_power_of_two(size_t capacity)
{
	return (capacity && !(capacity & (capacity - 1)));
}

static int circ_lifo_prepare(struct circ_lifo* const self, size_t capacity)
{
	ASSERT(self != NULL);

	if (self->dynamic)
	{
		self->buffer = calloc(capacity, sizeof(CIRC_LIFO_FORMAT));
		if (self->buffer == NULL)
			return -ENOMEM;

		return 0;
	}

	memset(self->buffer, 0.0f, capacity * sizeof(CIRC_LIFO_FORMAT));

	return 0;
}

int circ_lifo_construct(
	struct circ_lifo* const self,
	CIRC_LIFO_FORMAT* buffer,
	size_t capacity)
{
	ASSERT(self != NULL);

	self->dynamic = (buffer == NULL);

	if (!circ_lifo_is_power_of_two(capacity))
	{
		LOG_ERROR("Requested capacity isn't power of two");
		return -EPERM;
	}

	int error = circ_lifo_prepare(self, capacity);
	if (error < 0)
	{
		LOG_ERROR("Could not set circural lifo");
		return error;
	}

	self->i	   = 0u;
	self->mask = capacity - 1u;

	return 0;
}

void circ_lifo_destruct(struct circ_lifo* const self)
{
	ASSERT(self != NULL);

	circ_lifo_deallocate(self);
}

void circ_lifo_write(struct circ_lifo* const self, CIRC_LIFO_FORMAT data)
{
	ASSERT(self != NULL);

	self->i		      = (self->i - 1u) & self->mask;
	self->buffer[self->i] = data;
}

CIRC_LIFO_FORMAT circ_lifo_read(struct circ_lifo* const self)
{
	ASSERT(self != NULL);

	CIRC_LIFO_FORMAT newest_sample = self->buffer[self->i];
	self->i			       = (self->i + 1u) & self->mask;

	return newest_sample;
}
