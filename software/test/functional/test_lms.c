/**
 * @file    test_lms.c
 * @brief   <Provide some short description here>
 */

#include "dsp/fir.h"
#include "dsp/lms.h"
#include "signal/signal.h"
#include "utils/circ_fifo.h"
#include "utils/file_helpers.h"
#include "utils/logger.h"

#define FIR_W_FILE "test/functional/input/fir_dummy_14.csv"
#define LMS_W_FILE "test/functional/output/lms_w.csv"
#define LMS_E_FILE "test/functional/output/lms_e.csv"

#define SAMPLING_HZ    2 * 4000
#define NUM_OF_SAMPLES SAMPLING_HZ
#define ORDER          14u
#define FIFO_CAPACITY  1u << 14u
#define LMS_STEP       0.1

#ifndef TEST_FORMAT
#define TEST_FORMAT float
#endif

static TEST_FORMAT __x[NUM_OF_SAMPLES] = {0};
static TEST_FORMAT __d[NUM_OF_SAMPLES] = {0};
static TEST_FORMAT __y[NUM_OF_SAMPLES] = {0};
static TEST_FORMAT __e[NUM_OF_SAMPLES] = {0};

static TEST_FORMAT __w[ORDER]     = {0};
static TEST_FORMAT __w_est[ORDER] = {0};

static struct circ_fifo __x_p_cf;
static struct circ_fifo __x_w_cf;
static struct circ_fifo __x_lms_cf;

static int __test_dsp_prepare_x(void)
{
        struct signal sine_100_hz = {
                .amplitude          = 5.0,
                .frequency          = 100,
                .sampling_frequency = SAMPLING_HZ,
                .type               = SIGNAL_TYPE_SINE,
        };

        struct signal sine_300_hz = {
                .amplitude          = 1.5,
                .frequency          = 300,
                .sampling_frequency = SAMPLING_HZ,
                .type               = SIGNAL_TYPE_SINE,
        };

        struct signal noise      = {.amplitude = 1, .type = SIGNAL_TYPE_NOISE};
        struct signal* signals[] = {&sine_100_hz, &sine_300_hz, &noise};

        signal_initialize_noise();
        signal_generate_sum(signals, 3, __x, NUM_OF_SAMPLES);

        int error = circ_fifo_construct(&__x_p_cf, NULL, FIFO_CAPACITY);
        if (error < 0)
                return error;

        error = circ_fifo_construct(&__x_w_cf, NULL, FIFO_CAPACITY);
        if (error < 0)
                return error;

        error = circ_fifo_construct(&__x_lms_cf, NULL, FIFO_CAPACITY);
        if (error < 0)
                return error;

        for (size_t i = 0u; i < NUM_OF_SAMPLES; i++)
        {
                circ_fifo_write(&__x_p_cf, __x[i]);
                circ_fifo_write(&__x_w_cf, __x[i]);
                circ_fifo_write(&__x_lms_cf, __x[i]);
        }

        return error;
}

int test_dsp_lms(void)
{
        const int order = file_helpers_read_buf(FIR_W_FILE, __w);
        if (order != ORDER)
                return order;
        LOG_INFO("Loaded estimated filter coeffs with order %d", order);

        int error = __test_dsp_prepare_x();
        if (error < 0)
                return error;

        for (size_t i = 0u; i < NUM_OF_SAMPLES; i++)
        {
                error = file_helpers_write_buf(
                        LMS_W_FILE, ',', __w_est, ORDER, true);
                if (error < 0)
                        return error;

                __d[i] = dsp_fir(&__x_p_cf, __w, ORDER, i);
                __y[i] = dsp_fir(&__x_w_cf, __w_est, ORDER, i);

                __e[i] = __d[i] - __y[i];
                dsp_lms(__w_est, __e[i], LMS_STEP, &__x_lms_cf, order, i);
        }

        for (uint8_t i = 0; i < order; i++)
        {
                LOG_INFO("Filter coeff[%d] = %f", i, __w[i]);
                LOG_INFO("LSM Filter coeff[%d] = %f", i, __w_est[i]);
        }

        LOG_INFO("Last error = %f", __e[NUM_OF_SAMPLES - 1]);

        error = file_helpers_write_buf(
                LMS_E_FILE, '\n', __e, NUM_OF_SAMPLES, false);
        if (error < 0)
                return error;

        return 0;
}

int main(int argc, char* argv[])
{
        LOG_INFO("Running LMS test");
        int error = test_dsp_lms();
        if (error < 0)
                LOG_ERROR("LMS test failed");
        else
                LOG_INFO("LMS test ended with success");

        return error;
}
