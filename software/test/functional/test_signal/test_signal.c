/**
 * @file    test_signal.c
 * @brief   <Provide some short description here>
 */

#include "test_signal.h"

#include "signal/signal.h"
#include "utils/file_helpers.h"

#define SAMPLING_FREQUENCY 44100
#define NUM_OF_SAMPLES     SAMPLING_FREQUENCY

static float test_signal_buffer[NUM_OF_SAMPLES] = {0u};

int test_signal_dump_sine(void)
{
        struct signal sine_5_hz = {
                .amplitude          = 1.0f,
                .frequency          = 5.0f,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .type               = SIGNAL_TYPE_SINE,
        };

        signal_generate(&sine_5_hz, test_signal_buffer, NUM_OF_SAMPLES);
        return file_helpers_write_buf(
                "test/functional/output/sine_5_hz.csv",
                '\n',
                test_signal_buffer,
                NUM_OF_SAMPLES);
}

int test_signal_dump_triangle(void)
{
        struct signal triangle_10_hz = {
                .amplitude          = 1.0f,
                .frequency          = 10.0f,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .type               = SIGNAL_TYPE_TRIANGLE,
        };

        signal_generate(&triangle_10_hz, test_signal_buffer, NUM_OF_SAMPLES);
        return file_helpers_write_buf(
                "test/functional/output/triangle_10_hz.csv",
                '\n',
                test_signal_buffer,
                NUM_OF_SAMPLES);
}

int test_signal_dump_noise(void)
{
        struct signal noise = {
                .amplitude          = 2.0f,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .type               = SIGNAL_TYPE_NOISE,
        };

        signal_generate(&noise, test_signal_buffer, NUM_OF_SAMPLES);
        return file_helpers_write_buf(
                "test/functional/output/noise.csv",
                '\n',
                test_signal_buffer,
                NUM_OF_SAMPLES);
}

int test_signal_dump_sum(void)
{
        struct signal sine_5_hz = {
                .amplitude          = 2.0f,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .type               = SIGNAL_TYPE_SINE,
                .frequency          = 5.0f,
        };

        struct signal sine_10_hz = {
                .amplitude          = 1.0f,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .type               = SIGNAL_TYPE_SINE,
                .frequency          = 50.0f,
        };

        struct signal* signals[] = {&sine_5_hz, &sine_10_hz};

        signal_generate_sum(signals, 2, test_signal_buffer, NUM_OF_SAMPLES);
        return file_helpers_write_buf(
                "test/functional/output/sum.csv",
                '\n',
                test_signal_buffer,
                NUM_OF_SAMPLES);
}