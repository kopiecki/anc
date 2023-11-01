/**
 * @file    lms.c
 * @brief   NLMS implemenation
 */

#include "dsp/lms.h"
#include "utils/assert.h"
#include "utils/circ_fifo.h"
#include "utils/common.h"

void dsp_nlms(
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

        float div = 0.0;
        for (size_t i = 0u; i < order; i++) {
                div += circ_fifo_peek(cf, i) * circ_fifo_peek(cf, i);
        }

        const float mult = step * error / div;
        for (size_t i = 0u; i < order; i++) {
                w[i] += mult * circ_fifo_peek(cf, i + offset);
        }
}
