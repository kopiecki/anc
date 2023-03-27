/**
 * @file    ring_buffer.c
 * @brief   <Provide some short description here>
 */

#include "utils/ring_buffer.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include "logger/logger.h"
#include "utils/assert.h"

struct ring_buffer_s
{
        RING_BUFFER_FORMAT* buffer;
        size_t read_i, write_i;
        size_t capacity;
};

static inline void ring_buffer_deallocate(ring_buffer_t* const self)
{
        if (self->buffer != NULL)
                free(self->buffer);
        if (self != NULL)
                free(self);
}

static inline void ring_buffer_reset(ring_buffer_t* const self)
{
        ASSERT(self != NULL);
        self->read_i = self->write_i = 0u;
}

static inline bool ring_buffer_is_full(ring_buffer_t* const self)
{
        return ((self->write_i - self->read_i) & self->capacity)
               == self->capacity;
}

static inline bool ring_buffer_is_empty(ring_buffer_t* const self)
{
        return (self->write_i == self->read_i);
}

static inline size_t ring_buffer_num_of_elements(ring_buffer_t* const self)
{
        return (self->write_i - self->read_i) & self->capacity;
}

ring_buffer_t* const ring_buffer_construct(size_t capacity)
{
        ring_buffer_t* self = malloc(sizeof(*self));
        if (self == NULL)
                return NULL;

        self->capacity = --capacity;
        self->buffer   = malloc(self->capacity * sizeof(RING_BUFFER_FORMAT));
        if (self->buffer == NULL)
                goto bad_alloc;

        ring_buffer_reset(self);

        return self;

bad_alloc:
        ring_buffer_deallocate(self);
        return NULL;
}

void ring_buffer_destruct(ring_buffer_t* const self)
{
        ASSERT(self != NULL);
        ring_buffer_deallocate(self);
}

void ring_buffer_write(ring_buffer_t* const self, RING_BUFFER_FORMAT data)
{
        if (ring_buffer_is_full(self))
                self->read_i = (self->read_i + 1u) & self->capacity;

        self->buffer[self->write_i] = data;
        self->write_i = (self->write_i + 1) & self->capacity;
}

void ring_buffer_write_burst(
        ring_buffer_t* const self,
        const RING_BUFFER_FORMAT* const data,
        size_t size)
{
        ASSERT(self != NULL);
        ASSERT(data != NULL);

        if (size > self->capacity)
                LOG_WARNING(
                        "Request burst write size is larger than buffer "
                        "capacity, this will result of overriding data");
        for (size_t i = 0u; i < size; ++i)
                ring_buffer_write(self, data[i]);
}

int ring_buffer_read(ring_buffer_t* const self, RING_BUFFER_FORMAT* const data)
{
        ASSERT(self != NULL);
        ASSERT(data != NULL);

        if (ring_buffer_is_empty(self))
        {
                LOG_INFO("Buffer is empty, cannot read data");
                return -EPERM;
        }

        *data        = self->buffer[self->read_i];
        self->read_i = (self->read_i + 1u) & self->capacity;

        return 0;
}

int ring_buffer_read_burst(
        ring_buffer_t* const self,
        RING_BUFFER_FORMAT* const data,
        size_t size)
{
        if (ring_buffer_is_empty(self))
        {
                LOG_ERROR("Buffer is empty, cannot read data");
                return -EPERM;
        }

        RING_BUFFER_FORMAT* l_data = data;
        size_t cnt                 = 0u;
        while ((cnt < size) && !ring_buffer_read(self, l_data))
        {
                cnt++;
                l_data++;
        }

        return 0;
}

uint16_t* ring_buffer_read_burst_directly(ring_buffer_t* const self, size_t size)
{
        if (ring_buffer_is_empty(self))
        {
                LOG_INFO("Buffer is empty, cannot read data");
                return NULL;
        }

        if (size > self->capacity)
        {
                LOG_INFO("Requested read size is larger than buffer capacity");
                return NULL;
        }

        RING_BUFFER_FORMAT* read_ptr = &self->buffer[self->read_i];
        self->read_i                 = (self->read_i + size) & self->capacity;

        return read_ptr;
}
