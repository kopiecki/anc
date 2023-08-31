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
#define FIR_ORDER          32

static float signal_buffer[NUM_OF_SAMPLES] = {0u};
static float filtered_signal_buffer[NUM_OF_SAMPLES];
static float fir_imp_res_buffer[IMP_RES_MAX_SIZE];

static inline float signal_normalize(float y, float min, float max)
{
        return 2 * ((y - min) / (max - min)) - 1;
}

static int generate_signal(void)
{
        struct signal signals[100] = {0};

        for (size_t i = 0; i < 500; i += 5) {
                signals[i - 50].amplitude          = INT16_MAX;
                signals[i - 50].frequency          = i;
                signals[i - 50].phase              = 0;
                signals[i - 50].sampling_frequency = SAMPLING_FREQUENCY;
                signals[i - 50].type               = SIGNAL_TYPE_SINE;
        }

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

        for (size_t i = 0; i < NUM_OF_SAMPLES; i++) {
                circ_fifo_write(&cf, signal_buffer[i]);
                filtered_signal_buffer[i] =
                        dsp_fir(&cf, fir_imp_res_buffer, imp_res_size, 0);
        }

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
