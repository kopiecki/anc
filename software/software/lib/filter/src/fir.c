/**
 * @file    fir.c
 * @brief   <Provide some short description here>
 */

#include "filter/fir.h"

#include "logger/logger.h"
#include "utils/assert.h"
#include "utils/circ_fifo.h"

#define CF ((struct circ_fifo*)(cf))

float fir_filter(void* const cf, const float* const imp_res, size_t imp_res_size)
{
	ASSERT(cf != NULL);
	ASSERT(imp_res != NULL);

	float y = 0.0f;
	for (size_t i = 0u; i < imp_res_size; i++)
		y += imp_res[i] * circ_fifo_peek(CF, i);
	CF->i = (CF->i + 1u) & CF->mask;

	return y;
}
