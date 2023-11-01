/**
 * @file    circ_fifo.h
 * @brief   Circural fifo implementation
 */

#ifndef CIRC_FIFO_H_
#define CIRC_FIFO_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Circular fifo state.
 */
struct circ_fifo
{
        float* buffer;
        size_t mask;
        size_t i;
        bool dynamic;
};

/**
 * @brief Access data from circ fifo.
 */
#define circ_fifo_peek(cf, off) (cf->buffer[(cf->i + (off)) & cf->mask])

/**
 * @brief Circ fifo constructor.
 * 
 * @param self          State holder.
 * @param buffer        Preallocated buffer, NULL if dynamic
 * @param capacity      Allocation size
 * @return int          Error code, 0 on success.
 */
int circ_fifo_construct(
        struct circ_fifo* const self, float* const buffer, size_t capacity);

/**
 * @brief Circ fifo destructor
 * 
 * @param self          State holder.
 */
void circ_fifo_destruct(struct circ_fifo* const self);

/**
 * @brief Write to circ fifo with movement
 * 
 * @param self          State holder
 * @param data          Data to write
 */
void circ_fifo_write(struct circ_fifo* const self, float data);

/**
 * @brief Read from circ fifo with movement
 * 
 * @param self          State holder
 * @return float        Obtained data
 */
float circ_fifo_read(struct circ_fifo* const self);

/**
 * @brief Erase all data from circ fifo
 * 
 * @param self          State holder
 * @return float* 
 */
float* circ_fifo_purge(struct circ_fifo* const self);

static inline size_t circ_fifo_capacity(struct circ_fifo* const self)
{
        return self->mask + 1u;
}

#endif /* RING_BUFFER_H_ */
