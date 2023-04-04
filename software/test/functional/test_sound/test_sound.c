/**
 * @file    test_sound.c
 * @brief   <Provide some short description here>
 */

#include "test_sound.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "logger/logger.h"
#include "signal/signal.h"
#include "sound_card/sound_card.h"

#define SOUND_CARD_NAME    "plughw:1,0"
#define SAMPLING_FREQUENCY 44100
#define NUM_OF_SAMPLES     SAMPLING_FREQUENCY

static float test_sound_sine_buffer[NUM_OF_SAMPLES];
static sound_card_t* test_sound_sc;

static void test_sound_generate_sine(float* const sample_buffer, size_t size)
{
        struct signal sine = {
                .amplitude          = 1.0f,
                .frequency          = 200,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .type               = SIGNAL_TYPE_SINE,
        };

        signal_generate(&sine, sample_buffer, size);
}

static int test_sound_create_card(void)
{
        const struct sound_card_config sc_config = {
                .access         = SOUND_CARD_ACCESS_SEPARATE,
                .channels       = 2u,
                .name           = SOUND_CARD_NAME,
                .rate           = SAMPLING_FREQUENCY,
                .stream         = SOUND_CARD_STREAM_PLAYBACK,
                .buffer_time_us = 200000,
                .period_time_us = 100000,
                .poll           = true,
        };

        test_sound_sc = sound_card_construct(&sc_config);
        if (test_sound_sc == NULL)
                return -ENOTSUP;

        return 0;
}

static struct sound_card_task test_sound_card_create_task(void)
{
        const size_t period_size_bytes =
                sound_card_get_period_size_bytes(test_sound_sc);
        float* sample_buffer =
                calloc(2 * period_size_bytes / sizeof(float), sizeof(float));

        struct sound_card_task task = {
                .sample_buffers = {sample_buffer, sample_buffer},
                .num_of_periods = 2u,
                .size           = 2 * period_size_bytes / (sizeof(float)),
        };

        return task;
}

int test_sound_play_sine(void)
{
        test_sound_create_card();
        struct sound_card_task task = test_sound_card_create_task();
        test_sound_generate_sine(task.sample_buffers[0], task.size);

        sound_card_playback(test_sound_sc, &task);
        while (1)
                sound_card_playback(test_sound_sc, &task);
}

int test_sound_poll_playback_loop(void)
{
        test_sound_create_card();
        struct sound_card_task task = test_sound_card_create_task();

        int error  = 0;
        bool ready = false;
        while (1)
        {
                int error = sound_card_poll_is_ready(test_sound_sc, &ready);
                if (error < 0)
                        break;

                if (ready)
                {
                        test_sound_generate_sine(
                                task.sample_buffers[0], task.size);
                        error = sound_card_playback(test_sound_sc, &task);
                        if (error < 0)
                                ready = false;
                }
        }

        return error;
}
