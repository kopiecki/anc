/**
 * @file    lms.c
 * @brief   <Provide some short description here>
 */

#include "dsp/lms.h"

#include "utils/assert.h"
#include "utils/circ_fifo.h"

#define CF_X ((struct circ_fifo*)(cf_x))

void dsp_lms(
        DSP_FORMAT* const w,
        DSP_FORMAT error,
        DSP_FORMAT step,
        void* const cf_x,
        size_t order,
        size_t cnt)
{
        ASSERT(w != NULL);
        ASSERT(cf_x != NULL);

        for (size_t i = 0u; i < order; i++)
                w[i] += step * circ_fifo_peek(CF_X, i + cnt) * error;
        // CF_X->i = ((CF_X->i + 1u)) & CF_X->mask;
}
