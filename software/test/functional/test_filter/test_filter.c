/**
 * @file    test_signal.h
 * @brief   <Provide some short description here>
 */

#include "test_filter.h"

#include "filter/fir.h"
#include "signal/signal.h"
#include "utils/circ_fifo.h"
#include "utils/file_helpers.h"

#define CIRC_FIFO_CAPACITY (1u << 16u)
#define SAMPLING_FREQUENCY 500
#define NUM_OF_SAMPLES     SAMPLING_FREQUENCY
#define IMP_RES_MAX_SIZE   100
#define IMP_RES_FILE       "test/functional/input/imp_res.csv"
#define FILTER_ORDER       18

static float test_filter_signal_buffer[NUM_OF_SAMPLES];
static float test_filter_filtered_signal_buffer[NUM_OF_SAMPLES];
static float test_filter_imp_res_buffer[IMP_RES_MAX_SIZE];

static int test_filter_generate_signal(void)
{
        struct signal sine_5_hz = {
                .amplitude          = 1.0f,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .frequency          = 5.0f,
                .type               = SIGNAL_TYPE_SINE,
        };

        struct signal sine_100_hz = {
                .amplitude          = 0.2f,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .frequency          = 100.0f,
                .type               = SIGNAL_TYPE_SINE,
        };

        struct signal* signals[] = {&sine_5_hz, &sine_100_hz};
        signal_generate_sum(
                signals, 2, test_filter_signal_buffer, NUM_OF_SAMPLES);

        return file_helpers_write_buf(
                "test/functional/output/before_fir.csv",
                '\n',
                test_filter_signal_buffer,
                NUM_OF_SAMPLES);
}

int test_filter_fir()
{
        float* imp_res[FILTER_ORDER] = {0};
        struct circ_fifo cf;

        int error = circ_fifo_construct(&cf, NULL, CIRC_FIFO_CAPACITY);
        if (error < 0)
                return error;

        int imp_res_size =
                file_helpers_read_buf(IMP_RES_FILE, test_filter_imp_res_buffer);
        if (imp_res_size < 0)
                return error;

        error = test_filter_generate_signal();
        if (error < 0)
                return error;

        for (size_t i = 0u; i < NUM_OF_SAMPLES; i++)
                circ_fifo_write(&cf, test_filter_signal_buffer[i]);
        for (size_t i = NUM_OF_SAMPLES; i > 0; i--)
                test_filter_filtered_signal_buffer[i] = fir_filter(
                        &cf, test_filter_imp_res_buffer, imp_res_size);

        return file_helpers_write_buf(
                "test/functional/output/after_fir.csv",
                '\n',
                test_filter_filtered_signal_buffer,
                NUM_OF_SAMPLES);
}
