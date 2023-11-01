/**
 * @file    circ_fifo.c
 * @brief   Circural fifo implementation
 */

#include "utils/circ_fifo.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "utils/assert.h"
#include "utils/common.h"
#include "utils/logger.h"

static inline void circ_fifo_deallocate(struct circ_fifo* const self)
{
        ASSERT(self != NULL);

        if (self->dynamic)
                free(self->buffer);
}

static int circ_fifo_prepare(struct circ_fifo* const self, size_t capacity)
{
        ASSERT(self != NULL);

        if (self->dynamic) {
                self->buffer = calloc(capacity, sizeof(float));
                if (self->buffer == NULL)
                        return -ENOMEM;

                return 0;
        }

        memset(self->buffer, 0.0f, self->mask * sizeof(float));

        return 0;
}

int circ_fifo_construct(
        struct circ_fifo* const self, float* const buffer, size_t capacity)
{
        ASSERT(self != NULL);

        self->dynamic = (buffer == NULL);
        if (!IS_POW_2(capacity)) {
                LOG_ERROR("Requested capacity isn't power of two");
                return -EPERM;
        }

        self->buffer = buffer;

        int error = circ_fifo_prepare(self, capacity);
        if (error < 0) {
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

void circ_fifo_write(struct circ_fifo* const self, float data)
{
        self->i               = (self->i - 1u) & self->mask;
        self->buffer[self->i] = data;
}

float circ_fifo_read(struct circ_fifo* const self)
{
        ASSERT(self != NULL);

        float newest_sample = self->buffer[self->i];
        self->i             = (self->i + 1u) & self->mask;

        return newest_sample;
}
