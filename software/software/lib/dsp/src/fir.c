/**
 * @file    fir.c
 * @brief   <Provide some short description here>
 */

#include "dsp/fir.h"

#include "utils/assert.h"
#include "utils/circ_fifo.h"
#include "utils/logger.h"

#define CF ((struct circ_fifo*)(cf))

float dsp_fir(void* const cf, const DSP_FORMAT* const w, size_t order, size_t cnt)
{
        ASSERT(cf != NULL);
        ASSERT(w != NULL);

        DSP_FORMAT y = 0.0f;
        for (size_t i = 0u; i < order; i++)
                y += w[i] * circ_fifo_peek(CF, i + cnt);
        // CF->i = (CF->i + 1u) & CF->mask;

        return y;
}
