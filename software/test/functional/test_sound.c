/**
 * @file    test_sound.c
 * @brief   <Provide some short description here>
 */

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "signal/signal.h"
#include "sound_card/sound_card.h"
#include "utils/file_helpers.h"
#include "utils/logger.h"

#ifndef TEST_FORMAT
#define TEST_FORMAT float
#endif

#define CAPTURE_FILE_LOG_INPUT "capture_input.csv"
#define CAPTURE_FILE_LOG_ERROR "capture_error.csv"

#define SC_PLAYBACK_NAME   "plughw:1,0"
#define SC_CAPTURE_NAME    "plughw:1,0"
#define SAMPLING_FREQUENCY 4000
#define NUM_OF_SAMPLES     SAMPLING_FREQUENCY

#define BUFFER_TIME_US 2000000
#define PERIOD_TIME_US 1000000
#define CHANNELS       2

static sound_card_t* __sc_playback = NULL;
static sound_card_t* __sc_capture  = NULL;

static struct sound_card_task __playback_task;
static struct sound_card_task __capture_task;

static TEST_FORMAT* __noise_buffer;
static TEST_FORMAT* __antinoise_buffer;

static TEST_FORMAT* __input_buffer;
static TEST_FORMAT* __error_buffer;

#define NUM_OF_BUFFS 100
static TEST_FORMAT* __noise_buffers[NUM_OF_BUFFS];
static TEST_FORMAT* __error_buffers[NUM_OF_BUFFS];
static TEST_FORMAT* __input_buffers[NUM_OF_BUFFS];

static struct signal __noise = {
        .amplitude = 1.0f,
        .type      = SIGNAL_TYPE_NOISE,
};

size_t period_size;

enum playback_buffers
{
        NOISE_BUFFER     = 0,
        ANTINOISE_BUFFER = 1,
};

enum capture_buffers
{
        ERROR_BUFFER = 0,
        INPUT_BUFFER = 1,
};

static int __create_cards(void)
{
        const struct sound_card_config sc_playback_config = {
                .access         = SOUND_CARD_ACCESS_SEPARATE,
                .channels       = CHANNELS,
                .name           = SC_PLAYBACK_NAME,
                .rate           = SAMPLING_FREQUENCY,
                .stream         = SOUND_CARD_STREAM_PLAYBACK,
                .buffer_time_us = BUFFER_TIME_US,
                .period_time_us = PERIOD_TIME_US,
                .poll           = true,
        };

        const struct sound_card_config sc_capture_config = {
                .access         = SOUND_CARD_ACCESS_SEPARATE,
                .channels       = CHANNELS,
                .name           = SC_CAPTURE_NAME,
                .rate           = SAMPLING_FREQUENCY,
                .stream         = SOUND_CARD_STREAM_CAPTURE,
                .buffer_time_us = BUFFER_TIME_US,
                .period_time_us = PERIOD_TIME_US,
                .poll           = true,
        };

        __sc_playback = sound_card_construct(&sc_playback_config);
        if (__sc_playback == NULL)
                return -ENOTSUP;

        __sc_capture = sound_card_construct(&sc_capture_config);
        if (__sc_capture == NULL)
                return -ENOTSUP;

        sound_card_synchronize(__sc_capture, __sc_playback);

        LOG_INFO("Created sound cards");

        return 0;
}

static int __create_buffers(void)
{
        const size_t period_size_bytes_playback =
                sound_card_get_period_size_bytes(__sc_playback);
        const size_t period_size_bytes_capture =
                sound_card_get_period_size_bytes(__sc_capture);

        period_size =
                period_size_bytes_playback / (sizeof(TEST_FORMAT) * CHANNELS);

        __noise_buffer =
                calloc(period_size_bytes_playback / sizeof(TEST_FORMAT),
                       sizeof(TEST_FORMAT));
        if (__noise_buffer == NULL)
                return -ENOMEM;

        __antinoise_buffer =
                calloc(period_size_bytes_playback / sizeof(TEST_FORMAT),
                       sizeof(TEST_FORMAT));
        if (__antinoise_buffer == NULL)
                return -ENOMEM;

        __input_buffer =
                calloc(period_size_bytes_capture / sizeof(TEST_FORMAT),
                       sizeof(TEST_FORMAT));
        if (__input_buffer == NULL)
                return -ENOMEM;

        __error_buffer =
                calloc(period_size_bytes_capture / sizeof(TEST_FORMAT),
                       sizeof(TEST_FORMAT));
        if (__error_buffer == NULL)
                return -ENOMEM;

        for (size_t i = 0u; i < NUM_OF_BUFFS; i++)
        {
                __noise_buffers[i] =
                        calloc(period_size_bytes_capture / sizeof(TEST_FORMAT),
                               sizeof(TEST_FORMAT));
                __error_buffers[i] =
                        calloc(period_size_bytes_capture / sizeof(TEST_FORMAT),
                               sizeof(TEST_FORMAT));
                __input_buffers[i] =
                        calloc(period_size_bytes_capture / sizeof(TEST_FORMAT),
                               sizeof(TEST_FORMAT));
        }

        LOG_INFO("Created buffers");

        return 0;
}

void __create_playback_task(void)
{
        const size_t period_size_bytes =
                sound_card_get_period_size_bytes(__sc_playback);

        __playback_task.sample_buffers[NOISE_BUFFER]     = __noise_buffer;
        __playback_task.sample_buffers[ANTINOISE_BUFFER] = __antinoise_buffer;
        __playback_task.size = period_size_bytes / sizeof(float);

        LOG_INFO("Created playback task");
}

void __create_capture_task(void)
{
        const size_t period_size_bytes =
                sound_card_get_period_size_bytes(__sc_capture);

        struct sound_card_task task                 = {0};
        __capture_task.sample_buffers[INPUT_BUFFER] = __input_buffer;
        __capture_task.sample_buffers[ERROR_BUFFER] = __error_buffer;
        __capture_task.size = period_size_bytes / sizeof(float);

        LOG_INFO("Created capture task");
}

void __cleanup(void)
{
        if (__sc_capture != NULL)
                sound_card_destruct(__sc_capture);
        if (__sc_playback != NULL)
                sound_card_destruct(__sc_playback);
        if (__input_buffer != NULL)
                free(__input_buffer);
        if (__error_buffer != NULL)
                free(__error_buffer);
        if (__noise_buffer != NULL)
                free(__noise_buffer);
        if (__antinoise_buffer != NULL)
                free(__antinoise_buffer);
}

void* __playback_poll_loop(void*)
{
        LOG_INFO("Starting playback poll");

        bool ready = false;
        for (size_t i = 0; i < NUM_OF_BUFFS; i++)
        {
                int error = sound_card_poll_is_ready(__sc_playback, &ready);
                if (error < 0)
                        return NULL;

                if (ready)
                {
                        __playback_task.sample_buffers[NOISE_BUFFER] =
                                __noise_buffers[i];
                        error = sound_card_playback(
                                __sc_playback, &__playback_task);
                        if (error < 0)
                                ready = false;
                }
        }

        LOG_INFO("Finished playback poll");
}

void* __capture_poll_loop(void*)
{
        LOG_INFO("Starting capture poll");

        bool ready = false;
        for (size_t i = 0; i < NUM_OF_BUFFS; i++)
        {
                int error = sound_card_poll_is_ready(__sc_capture, &ready);
                if (error < 0)
                        return NULL;

                if (ready)
                {
                        __capture_task.sample_buffers[ERROR_BUFFER] =
                                __error_buffers[i];
                        __capture_task.sample_buffers[INPUT_BUFFER] =
                                __input_buffers[i];
                        sound_card_capture(__sc_capture, &__capture_task);
                        if (error < 0)
                                ready = false;
                }
        }

        LOG_INFO("Finished capture poll");
}

int __test_loop(void)
{
        int error = __create_cards();
        if (error < 0)
        {
                LOG_ERROR("Cannot create cards!");
                __cleanup();
        }

        error = __create_buffers();
        if (error < 0)
        {
                LOG_ERROR("Cannot create buffers!");
                __cleanup();
        }

        __create_playback_task();
        __create_capture_task();

        for (size_t i = 0; i < NUM_OF_BUFFS; i++)
                signal_generate(&__noise, __noise_buffers[i], period_size);

        pthread_t capture_thread;
        pthread_t playback_thread;

        pthread_create(&playback_thread, NULL, __playback_poll_loop, NULL);
        pthread_create(&capture_thread, NULL, __capture_poll_loop, NULL);

        pthread_join(playback_thread, NULL);
        pthread_join(capture_thread, NULL);

        for (size_t i = 0; i < NUM_OF_BUFFS; i++)
        {
                file_helpers_write_buf(
                        "capture.csv", '\n', __error_buffers[i], period_size, true);
                file_helpers_write_buf(
                        "input.csv", '\n', __noise_buffers[i], period_size, true);
        }

        return 0;
}

int main(int argc, char* argv[])
{
        LOG_INFO("Running sound test");
        int error = __test_loop();
        if (error < 0)
                LOG_ERROR("Sound test failed");
        else
                LOG_INFO("Sound test ended with success");

        __cleanup();

        return error;
}
