/**
 * @file    fir.c
 * @brief   <Provide some short description here>
 */

#include "dsp/fir.h"

#include "utils/assert.h"
#include "utils/circ_fifo.h"
#include "utils/common.h"
#include "utils/logger.h"

float dsp_fir(
        struct circ_fifo* const cf,
        const float* restrict const w,
        size_t order,
        size_t offset)
{
        ASSERT(cf != NULL);
        ASSERT(w != NULL);
        ASSERT(IS_POW_2(order));

        /**
         * @brief Loop unrolling to optimize processing time.
         */
        float y, y0, y1, y2, y3 = 0.0f;
        for (size_t i = 0u; i < order; i++) {
                y = w[i] * circ_fifo_peek(cf, i + offset);
        }

        return y;
}
