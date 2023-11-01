/**
 * @file    fir.c
 * @brief   FIR implementation
 */

#include "dsp/fir.h"

#include "utils/circ_fifo.h"

static float __fir(
        const float* restrict x,
        const float* restrict w,
        unsigned int i,
        unsigned int mask,
        unsigned int n)
{
        float y = 0.0f;
        for (unsigned j = 0u; j < n; j++)
                y += w[j] * x[i + j];
        return y;
}

float dsp_fir(
        struct circ_fifo* const cf,
        const float* restrict const w,
        unsigned int order,
        unsigned int offset)
{
        return __fir(cf->buffer, w, cf->i, cf->mask, order);
}
