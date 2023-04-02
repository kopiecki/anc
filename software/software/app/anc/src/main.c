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
#include "utils/circ_fifo.h"
#include "utils/common.h"
#include "utils/file_helpers.h"

#define SOUND_CARD_NAME       "plughw:1,0"
#define SAMPLING_FREQUEUCY    44100
#define LOW_SINE_FREQUENCY    100
#define MEDIUM_SINE_FREQUENCY 80
#define NUM_OF_SAMPLES        SAMPLING_FREQUEUCY

static const struct sound_card_config sc_config = {
        .name           = SOUND_CARD_NAME,
        .access         = SOUND_CARD_ACCESS_SEPARATE,
        .channels       = 2u,
        .rate           = SAMPLING_FREQUEUCY,
        .buffer_time_us = 20000,
        .period_time_us = 10000,
};

static sound_card_t* create_playback_sound_card(void)
{
        struct sound_card_config playback_config = sc_config;
        playback_config.stream                   = SOUND_CARD_STREAM_PLAYBACK;

        return sound_card_construct(&playback_config);
}

static sound_card_t* create_capture_sound_card(void)
{
        struct sound_card_config capture_config = sc_config;
        capture_config.stream                   = SOUND_CARD_STREAM_CAPTURE;

        return sound_card_construct(&capture_config);
}

int main(int argc, char* argv[])
{
        sound_card_t* const playback_sc = create_playback_sound_card();
        sound_card_t* const capture_sc  = create_capture_sound_card();

        sound_card_synchronize(playback_sc, capture_sc);

        const size_t period_size_bytes =
                sound_card_get_period_size_bytes(playback_sc);

        float* first_channel_buffer =
                calloc(period_size_bytes / sizeof(float), sizeof(float));
        float* second_channel_buffer =
                calloc(period_size_bytes / sizeof(float), sizeof(float));

        struct signal sine_100_hz = {
                .type               = SIGNAL_TYPE_SINE,
                .frequency          = 100,
                .sampling_frequency = SAMPLING_FREQUEUCY,
                .amplitude          = 1.0f,
        };

        struct signal sine_200_hz = {
                .type               = SIGNAL_TYPE_SINE,
                .frequency          = 200,
                .sampling_frequency = SAMPLING_FREQUEUCY,
                .amplitude          = 1.0f,
        };

        signal_generate(
                &sine_100_hz,
                first_channel_buffer,
                period_size_bytes / sizeof(float));

        signal_generate(
                &sine_200_hz,
                second_channel_buffer,
                period_size_bytes / sizeof(float));

        struct sound_card_task playback_task = {
                .sample_buffers = {first_channel_buffer, second_channel_buffer},
                .num_of_periods = 1u,
        };

        while (1)
        {
                sound_card_capture(capture_sc, &playback_task);
                sound_card_playback(playback_sc, &playback_task);
        }

        sound_card_destruct(playback_sc);
        sound_card_destruct(capture_sc);

        return 0;
}
