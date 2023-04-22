/**
 * @file    test_fir.c
 * @brief   <Provide some short description here>
 */

#include "dsp/fir.h"
#include "signal/signal.h"
#include "utils/circ_fifo.h"
#include "utils/file_helpers.h"
#include "utils/logger.h"

#define CIRC_FIFO_CAPACITY (1u << 16u)
#define SAMPLING_FREQUENCY 4000
#define NUM_OF_SAMPLES     SAMPLING_FREQUENCY
#define IMP_RES_MAX_SIZE   100
#define IMP_RES_FILE       "test/functional/input/fir_coeffs_26.csv"
#define FIR_ORDER          26

static float signal_buffer[NUM_OF_SAMPLES];
static float filtered_signal_buffer[NUM_OF_SAMPLES];
static float fir_imp_res_buffer[IMP_RES_MAX_SIZE];

static int generate_signal(void)
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
                .frequency          = 500.0f,
                .type               = SIGNAL_TYPE_SINE,
        };

        struct signal* signals[] = {&sine_5_hz, &sine_100_hz};
        signal_generate_sum(signals, 2, signal_buffer, NUM_OF_SAMPLES);

        return file_helpers_write_buf(
                "test/functional/output/before_fir.csv",
                '\n',
                signal_buffer,
                NUM_OF_SAMPLES,
                false);
}

static int fir(void)
{
        float* imp_res[FIR_ORDER] = {0};
        struct circ_fifo cf;

        int error = circ_fifo_construct(&cf, NULL, CIRC_FIFO_CAPACITY);
        if (error < 0)
                return error;

        int imp_res_size =
                file_helpers_read_buf(IMP_RES_FILE, fir_imp_res_buffer);
        if (imp_res_size < 0)
                return error;

        error = generate_signal();
        if (error < 0)
                return error;

        for (size_t i = 0u; i < NUM_OF_SAMPLES; i++)
                circ_fifo_write(&cf, signal_buffer[i]);
        for (size_t i = 0; i < NUM_OF_SAMPLES; i++)
                filtered_signal_buffer[NUM_OF_SAMPLES - 1 - i] =
                        dsp_fir(&cf, fir_imp_res_buffer, imp_res_size, i);

        return file_helpers_write_buf(
                "test/functional/output/after_fir.csv",
                '\n',
                filtered_signal_buffer,
                NUM_OF_SAMPLES,
                false);
}

int main(void)
{
        LOG_INFO("Running FIR test");
        int error = fir();
        if (error < 0)
                LOG_ERROR("FIR test failed");
        else
                LOG_INFO("FIR test ended with success");

        return error;
}
