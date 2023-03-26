/**
 * @file    ring_buffer.h
 * @brief   <Provide some short description here>
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <stddef.h>
#include <stdint.h>

#ifndef RING_BUFFER_FORMAT
#define RING_BUFFER_FORMAT uint16_t
#endif

typedef struct ring_buffer_s ring_buffer_t;

ring_buffer_t* const ring_buffer_construct(size_t capacity);
void ring_buffer_destruct(ring_buffer_t* const self);

void ring_buffer_write(ring_buffer_t* const self, RING_BUFFER_FORMAT data);
void ring_buffer_write_burst(
        ring_buffer_t* const self,
        const RING_BUFFER_FORMAT* const source,
        size_t size);

int ring_buffer_read(ring_buffer_t* const self, RING_BUFFER_FORMAT* const data);
int ring_buffer_read_burst(
        ring_buffer_t* const self,
        RING_BUFFER_FORMAT* const data,
        size_t size);

uint16_t* const ring_buffer_read_burst_directly(
        ring_buffer_t* const self,
        size_t size);

#endif /* RING_BUFFER_H_ */
