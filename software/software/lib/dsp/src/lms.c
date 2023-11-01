/**
 * @file    lms.c
 * @brief   LMS implementation
 */

#include "dsp/lms.h"

#include "utils/assert.h"
#include "utils/circ_fifo.h"
#include "utils/common.h"

void dsp_lms(
        float* restrict const w,
        float error,
        float step,
        struct circ_fifo* const cf,
        size_t order,
        size_t offset)
{
        ASSERT(w != NULL);
        ASSERT(cf != NULL);
        ASSERT(IS_POW_2(order));

        float mult = step * error;
        for (size_t i = 0u; i < order; i++) {
                w[i] += mult * circ_fifo_peek(cf, i + offset);
        }
}
