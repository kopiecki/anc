/**
 * @file    main.c
 * @brief   <Provide some short description here>
 */

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "logger/logger.h"
#include "signal/signal.h"
#include "sound_card/sound_card.h"
#include "utils/common.h"
#include "utils/file_helpers.h"

#define TEST_FILE_PATH "test/functional/output"

static int signal_test(void)
{
        LOG_INFO(" ------ Running signal test ------ ");

        const float signal_frequency = 44100;
        const int signal_size        = signal_frequency;
        float signal_buffer[signal_size];

        struct signal sine_5_hz = {
                .amplitude          = 1.0f,
                .frequency          = 5,
                .sampling_frequency = 44100,
                .type               = SIGNAL_TYPE_SINE,
        };

        signal_generate(&sine_5_hz, signal_buffer, signal_size);
        return file_helpers_write_buf(
                "test/functional/output/sine_100_hz.csv",
                '\n',
                signal_buffer,
                signal_size);
}

static int filter_test(void)
{
        LOG_INFO("Running filter test");
}

static int audio_test(void)
{
        LOG_INFO("Running audio test");
}

static int (*tests[])(void) = {
        signal_test,
        filter_test,
        audio_test,
};

static const char* TEST_TYPES[] = {
        "signal",
        "filter",
        "audio",
};

int main(int argc, char* argv[])
{
        if (argc != 2)
                return -EINVAL;

        for (uint8_t i = 0; i < ARRAY_SIZE(TEST_TYPES); i++)
                if (!strcmp(TEST_TYPES[i], argv[1]))
                        return tests[i]();
}
