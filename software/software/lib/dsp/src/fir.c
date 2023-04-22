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
        struct circ_fifo* const cf, const float* const w, size_t order, size_t offset)
{
        ASSERT(cf != NULL);
        ASSERT(w != NULL);
        ASSERT(IS_POW_2(order));

        /**
         * @brief Loop unrolling to optimize processing time.
         */
        float y0 = 0.0f;
        float y1 = 0.0f;
        float y2 = 0.0f;
        float y3 = 0.0f;
        for (size_t i = 0u; i < order; i += 4u) {
                y0 += w[i + 0] * circ_fifo_peek(cf, i + offset + 0);
                y1 += w[i + 1] * circ_fifo_peek(cf, i + offset + 1);
                y2 += w[i + 2] * circ_fifo_peek(cf, i + offset + 2);
                y3 += w[i + 3] * circ_fifo_peek(cf, i + offset + 3);
        }

        y0 += y1;
        y2 += y3;
        y0 += y2;

        return y0;
}
