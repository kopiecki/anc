/**
 * @file    test_signal.c
 * @brief   <Provide some short description here>
 */

#include "signal/signal.h"
#include "utils/file_helpers.h"
#include "utils/logger.h"

#define SAMPLING_FREQUENCY 44100
#define NUM_OF_SAMPLES     SAMPLING_FREQUENCY

static float signal_buffer[NUM_OF_SAMPLES] = {0};

static int dump_sine(void)
{
        struct signal sine_5_hz = {
                .amplitude          = 1,
                .frequency          = 5,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .type               = SIGNAL_TYPE_SINE,
                .phase              = 0,
        };

        signal_generate(&sine_5_hz, signal_buffer, NUM_OF_SAMPLES);
        return file_helpers_write_buf(
                "test/functional/output/sine_5_hz.csv",
                '\n',
                signal_buffer,
                NUM_OF_SAMPLES,
                false);
}

static int dump_triangle(void)
{
        struct signal triangle_10_hz = {
                .amplitude          = 1,
                .frequency          = 10,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .type               = SIGNAL_TYPE_TRIANGLE,
                .phase              = 0,
        };

        signal_generate(&triangle_10_hz, signal_buffer, NUM_OF_SAMPLES);
        return file_helpers_write_buf(
                "test/functional/output/triangle_10_hz.csv",
                '\n',
                signal_buffer,
                NUM_OF_SAMPLES,
                false);
}

static int dump_noise(void)
{
        struct signal noise = {
                .amplitude          = 2,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .type               = SIGNAL_TYPE_NOISE,
                .phase              = 0,
        };

        signal_generate(&noise, signal_buffer, NUM_OF_SAMPLES);
        return file_helpers_write_buf(
                "test/functional/output/noise.csv",
                '\n',
                signal_buffer,
                NUM_OF_SAMPLES,
                false);
}

static int dump_sum(void)
{
        struct signal sine_5_hz = {
                .amplitude          = 2,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .type               = SIGNAL_TYPE_SINE,
                .frequency          = 5,
                .phase              = 0,
        };

        struct signal sine_10_hz = {
                .amplitude          = 1,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .type               = SIGNAL_TYPE_SINE,
                .frequency          = 50.0,
                .phase              = 0,
        };

        struct signal* signals[] = {&sine_5_hz, &sine_10_hz};

        signal_generate_sum(signals, 2, signal_buffer, NUM_OF_SAMPLES);
        return file_helpers_write_buf(
                "test/functional/output/sum.csv",
                '\n',
                signal_buffer,
                NUM_OF_SAMPLES,
                false);
}

int main(void)
{
        LOG_INFO("Running signal test");

        int error = dump_sine();
        if (error < 0)
                return error;

        error = dump_triangle();
        if (error < 0)
                return error;

        error = dump_noise();
        if (error < 0)
                return error;

        error = dump_sum();
        if (error < 0)
                return error;

        LOG_INFO("Signal test ended");

        return 0;
}
