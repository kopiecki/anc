/**
 * @file    circ_fifo.c
 * @brief   <Provide some short description here>
 */

#include "utils/circ_fifo.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "logger/logger.h"
#include "utils/assert.h"

static inline void circ_fifo_deallocate(struct circ_fifo* const self)
{
        ASSERT(self != NULL);

        if (self->dynamic)
                free(self->buffer);
}

static inline bool circ_fifo_is_power_of_two(size_t capacity)
{
        return (capacity && !(capacity & (capacity - 1)));
}

static int circ_fifo_prepare(struct circ_fifo* const self, size_t capacity)
{
        ASSERT(self != NULL);

        if (self->dynamic)
        {
                self->buffer = calloc(capacity, sizeof(CIRC_FIFO_FORMAT));
                if (self->buffer == NULL)
                        return -ENOMEM;

                return 0;
        }

        memset(self->buffer, 0.0f, capacity * sizeof(CIRC_FIFO_FORMAT));

        return 0;
}

int circ_fifo_construct(
        struct circ_fifo* const self,
        CIRC_FIFO_FORMAT* buffer,
        size_t capacity)
{
        ASSERT(self != NULL);

        self->dynamic = (buffer == NULL);
        if (!circ_fifo_is_power_of_two(capacity))
        {
                LOG_ERROR("Requested capacity isn't power of two");
                return -EPERM;
        }

        int error = circ_fifo_prepare(self, capacity);
        if (error < 0)
        {
                LOG_ERROR("Could not set circural fifo");
                return error;
        }

        self->i    = 0u;
        self->mask = capacity - 1u;

        return 0;
}

void circ_fifo_destruct(struct circ_fifo* const self)
{
        ASSERT(self != NULL);

        circ_fifo_deallocate(self);
}

void circ_fifo_write(struct circ_fifo* const self, CIRC_FIFO_FORMAT data)
{
        ASSERT(self != NULL);

        self->i               = (self->i - 1u) & self->mask;
        self->buffer[self->i] = data;
}

CIRC_FIFO_FORMAT circ_fifo_read(struct circ_fifo* const self)
{
        ASSERT(self != NULL);

        CIRC_FIFO_FORMAT newest_sample = self->buffer[self->i];
        self->i                        = (self->i + 1u) & self->mask;

        return newest_sample;
}

CIRC_FIFO_FORMAT* circ_fifo_purge(struct circ_fifo* const self)
{
}
