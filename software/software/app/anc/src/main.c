/**
 * @file    main.c
 * @brief   <Provide some short description here>
 */

#define __FORCE_DEBUG_SEVERITY LOGGER_DEBUG_SEVERITY_TRACE

#include <errno.h>
#include <stdlib.h>

#include "filter/fir.h"
#include "logger/logger.h"
#include "signal/signal.h"
#include "sound_card/sound_card.h"
#include "utils/common.h"
#include "utils/file_helpers.h"

#define SOUND_CARD_NAME	      "plughw:1,0"
#define SAMPLING_FREQUEUCY    500
#define LOW_SINE_FREQUENCY    10
#define MEDIUM_SINE_FREQUENCY 80
#define NUM_OF_SAMPLES	      SAMPLING_FREQUEUCY

int main(int argc, char* argv[])
{
	float* sample_buffer = calloc(NUM_OF_SAMPLES, sizeof(float));
	float* fir_output    = calloc(NUM_OF_SAMPLES, sizeof(float));
	float* imp_res	     = calloc(NUM_OF_SAMPLES, sizeof(float));

	struct signal sine_100_hz = {
		.type		    = SIGNAL_TYPE_SINE,
		.frequency	    = LOW_SINE_FREQUENCY,
		.sampling_frequency = SAMPLING_FREQUEUCY,
		.amplitude	    = 1.0f,
	};

	struct signal sine_high = {
		.type		    = SIGNAL_TYPE_SINE,
		.frequency	    = MEDIUM_SINE_FREQUENCY,
		.sampling_frequency = SAMPLING_FREQUEUCY,
		.amplitude	    = 0.3f,
	};

	struct signal* signals[] = {&sine_100_hz, &sine_high};
	signal_generate_sum(signals, 2, sample_buffer, NUM_OF_SAMPLES);

	file_helpers_write_buf(
		"before_fir.csv", '\n', sample_buffer, NUM_OF_SAMPLES);
	int imp_res_size = file_helpers_read_buf(
		"/home/dk/projects/thesis/anc/software/imp_res.csv",
		imp_res);

	struct circ_lifo cl = {0u};
	circ_lifo_construct(&cl, 1 << 24u, true);

	for (size_t i = 0; i < NUM_OF_SAMPLES; i++)
		circ_lifo_write(&cl, sample_buffer[i]);

	for (size_t i = NUM_OF_SAMPLES - 1; i > 0; i--)
		fir_output[i] = fir_filter(&cl, imp_res, imp_res_size);

	file_helpers_write_buf(
		"after_fir.csv", '\n', fir_output, NUM_OF_SAMPLES);

	free(sample_buffer);
	free(fir_output);
	free(imp_res);
	circ_lifo_destruct(&cl);

	return 0;
}
