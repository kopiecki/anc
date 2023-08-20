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
#define ORDER          (size_t)14u
#define FIFO_CAPACITY  1u << 14u
#define LMS_STEP       0.1f

static float x[NUM_OF_SAMPLES] = {0};
static float d[NUM_OF_SAMPLES] = {0};
static float y[NUM_OF_SAMPLES] = {0};
static float e[NUM_OF_SAMPLES] = {0};

static float w[ORDER]     = {0};
static float w_est[ORDER] = {0};

static struct circ_fifo x_p_cf;
static struct circ_fifo x_w_cf;
static struct circ_fifo x_lms_cf;

static int prepare_x(void)
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
        signal_generate_sum(signals, 3, x, NUM_OF_SAMPLES);

        int error = circ_fifo_construct(&x_p_cf, NULL, FIFO_CAPACITY);
        if (error < 0)
                return error;

        error = circ_fifo_construct(&x_w_cf, NULL, FIFO_CAPACITY);
        if (error < 0)
                return error;

        error = circ_fifo_construct(&x_lms_cf, NULL, FIFO_CAPACITY);
        if (error < 0)
                return error;

        return error;
}

int lms(void)
{
        const int order = file_helpers_read_buf(FIR_W_FILE, w);
        if (order != ORDER)
                return order;
        LOG_INFO("Loaded estimated filter coeffs with order %d", order);

        int error = prepare_x();
        if (error < 0)
                return error;

        for (size_t i = 0u; i < NUM_OF_SAMPLES; i++) {
                error = file_helpers_write_buf(
                        LMS_W_FILE, ',', w_est, ORDER, true);
                if (error < 0)
                        return error;

                circ_fifo_write(&x_p_cf, x[i]);
                circ_fifo_write(&x_w_cf, x[i]);
                circ_fifo_write(&x_lms_cf, x[i]);

                d[i] = dsp_fir(&x_p_cf, w, ORDER, 0);
                y[i] = dsp_fir(&x_w_cf, w_est, ORDER, 0);

                e[i] = d[i] - y[i];
                dsp_lms(w_est, e[i], LMS_STEP, &x_lms_cf, ORDER, 0);
        }

        error = file_helpers_write_buf(
                LMS_E_FILE, '\n', e, NUM_OF_SAMPLES, false);
        if (error < 0)
                return error;

        return 0;
}

int main(void)
{
        LOG_INFO("Running LMS test");
        int error = lms();
        if (error < 0)
                LOG_ERROR("LMS test failed");
        else
                LOG_INFO("LMS test ended with success");

        return error;
}
