/**
 * @file    main.c
 * @brief   <Provide some short description here>
 */

#define __FORCE_DEBUG_SEVERITY LOGGER_DEBUG_SEVERITY_TRACE

#include <errno.h>
#include <stdlib.h>

#include "logger/logger.h"
#include "signal/signal.h"
#include "sound_card/sound_card.h"
#include "utils/file_helpers.h"

#define SOUND_CARD_NAME "plughw:1,0"
#define SINE_FREQUENCY  100

int main(int argc, char* argv[])
{
        const struct sound_card_config config = {
                .name           = SOUND_CARD_NAME,
                .format         = SOUND_CARD_FORMAT_U16,
                .rate           = 44100,
                .channels       = 2,
                .stream         = SOUND_CARD_STREAM_PLAYBACK,
                .buffer_time_us = 1000000,
                .period_time_us = 500000,
        };

        sound_card_t* const sound_card = sound_card_init(&config);
        if (sound_card == NULL)
                return -ENOMEM;

        const size_t period_size =
                sound_card_get_period_size_bytes(sound_card) / sizeof(uint16_t);

        struct sound_card_playback_task task = {
                .size           = period_size,
                .num_of_periods = 2,
        };

        task.sample_buffers[0] = calloc(period_size, sizeof(uint16_t));
        task.sample_buffers[1] = calloc(period_size, sizeof(uint16_t));


        struct signal square_100_hz = {
                .type               = SIGNAL_TYPE_TRIANGLE,
                .frequency          = SINE_FREQUENCY,
                .sampling_frequency = 44100,
        };

        struct signal triangle_100_hz = {
                .type               = SIGNAL_TYPE_SINE,
                .frequency          = SINE_FREQUENCY,
                .sampling_frequency = 44100,
        };

        struct signal saw_100_hz = {
                .type               = SIGNAL_TYPE_SAW,
                .frequency          = SINE_FREQUENCY,
                .sampling_frequency = 44100,
        };

        struct signal noise = {
                .type = SIGNAL_TYPE_NOISE,
        };

        signal_generate(&square_100_hz, task.sample_buffers[0], period_size);
        signal_generate(&triangle_100_hz, task.sample_buffers[1], period_size);

        sound_card_playback(sound_card, &task);
        sound_card_deinit(sound_card);

        signal_generate(&saw_100_hz, task.sample_buffers[0], period_size);

        file_helpers_write_buf(
                "saw.csv", '\n', task.sample_buffers[0], period_size);

        return 0;
}
