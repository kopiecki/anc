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

#include "dsp/fir.h"
#include "dsp/lms.h"
#include "signal/signal.h"
#include "sound_card/sound_card.h"
#include "utils/circ_fifo.h"
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

#define BUFFER_TIME_US 4000
#define PERIOD_TIME_US 2000
#define CHANNELS       2

static sound_card_t* sc_playback = NULL;
static sound_card_t* sc_capture  = NULL;

static struct sound_card_task playback_task;
static struct sound_card_task capture_task;

static TEST_FORMAT* noise_buffer;
static TEST_FORMAT* antinoise_buffer;
static TEST_FORMAT* antinoise_buffer_tmp;

static TEST_FORMAT* input_buffer;
static TEST_FORMAT* error_buffer;

#define NUM_OF_BUFFS 10000
static TEST_FORMAT* noise_buffers[NUM_OF_BUFFS];
static TEST_FORMAT* error_buffers[NUM_OF_BUFFS];
static TEST_FORMAT* input_buffers[NUM_OF_BUFFS];

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

        sc_playback = sound_card_construct(&sc_playback_config);
        if (sc_playback == NULL)
                return -ENOTSUP;

        sc_capture = sound_card_construct(&sc_capture_config);
        if (sc_capture == NULL)
                return -ENOTSUP;

        sound_card_synchronize(sc_capture, sc_playback);

        LOG_INFO("Created sound cards");

        return 0;
}

static int __create_buffers(void)
{
        const size_t period_size_bytes_playback =
                sound_card_get_period_size_bytes(sc_playback);
        const size_t period_size_bytes_capture =
                sound_card_get_period_size_bytes(sc_capture);

        period_size =
                period_size_bytes_playback / (sizeof(TEST_FORMAT) * CHANNELS);

        noise_buffer =
                calloc(period_size_bytes_playback / sizeof(TEST_FORMAT),
                       sizeof(TEST_FORMAT));
        if (noise_buffer == NULL)
                return -ENOMEM;

        antinoise_buffer =
                calloc(period_size_bytes_playback / sizeof(TEST_FORMAT),
                       sizeof(TEST_FORMAT));
        if (antinoise_buffer == NULL)
                return -ENOMEM;

        input_buffer =
                calloc(period_size_bytes_capture / sizeof(TEST_FORMAT),
                       sizeof(TEST_FORMAT));
        if (input_buffer == NULL)
                return -ENOMEM;

        error_buffer =
                calloc(period_size_bytes_capture / sizeof(TEST_FORMAT),
                       sizeof(TEST_FORMAT));
        if (error_buffer == NULL)
                return -ENOMEM;

        for (size_t i = 0u; i < NUM_OF_BUFFS; i++)
        {
                noise_buffers[i] =
                        calloc(period_size_bytes_capture / sizeof(TEST_FORMAT),
                               sizeof(TEST_FORMAT));
                error_buffers[i] =
                        calloc(period_size_bytes_capture / sizeof(TEST_FORMAT),
                               sizeof(TEST_FORMAT));
                input_buffers[i] =
                        calloc(period_size_bytes_capture / sizeof(TEST_FORMAT),
                               sizeof(TEST_FORMAT));
        }

        LOG_INFO("Created buffers");

        return 0;
}

void __create_playback_task(void)
{
        const size_t period_size_bytes =
                sound_card_get_period_size_bytes(sc_playback);

        playback_task.sample_buffers[NOISE_BUFFER]     = noise_buffer;
        playback_task.sample_buffers[ANTINOISE_BUFFER] = antinoise_buffer;
        playback_task.size = period_size_bytes / sizeof(float);

        LOG_INFO("Created playback task");
}

void __create_capture_task(void)
{
        const size_t period_size_bytes =
                sound_card_get_period_size_bytes(sc_capture);

        struct sound_card_task task               = {0};
        capture_task.sample_buffers[INPUT_BUFFER] = input_buffer;
        capture_task.sample_buffers[ERROR_BUFFER] = error_buffer;
        capture_task.size = period_size_bytes / sizeof(float);

        LOG_INFO("Created capture task");
}

void __cleanup(void)
{
        if (sc_capture != NULL)
                sound_card_destruct(sc_capture);
        if (sc_playback != NULL)
                sound_card_destruct(sc_playback);
        if (input_buffer != NULL)
                free(input_buffer);
        if (error_buffer != NULL)
                free(error_buffer);
        if (noise_buffer != NULL)
                free(noise_buffer);
        if (antinoise_buffer != NULL)
                free(antinoise_buffer);
}

void* __playback_poll_loop(void*)
{
        LOG_INFO("Starting playback poll");

        bool ready = false;
        for (size_t i = 0; i < NUM_OF_BUFFS; i++)
        {
                int error = sound_card_poll_is_ready(sc_playback, &ready);
                if (error < 0)
                        return NULL;

                if (ready)
                {
                        playback_task.sample_buffers[NOISE_BUFFER] =
                                noise_buffers[i];
                        error = sound_card_playback(sc_playback, &playback_task);
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
                int error = sound_card_poll_is_ready(sc_capture, &ready);
                if (error < 0)
                        return NULL;

                if (ready)
                {
                        capture_task.sample_buffers[ERROR_BUFFER] =
                                error_buffers[i];
                        capture_task.sample_buffers[INPUT_BUFFER] =
                                input_buffers[i];
                        sound_card_capture(sc_capture, &capture_task);
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
                signal_generate(&__noise, noise_buffers[i], period_size);

        pthread_t capture_thread;
        pthread_t playback_thread;

        pthread_create(&playback_thread, NULL, __playback_poll_loop, NULL);
        pthread_create(&capture_thread, NULL, __capture_poll_loop, NULL);

        pthread_join(playback_thread, NULL);
        pthread_join(capture_thread, NULL);

        for (size_t i = 0; i < NUM_OF_BUFFS; i++)
        {
                file_helpers_write_buf(
                        "capture.csv", '\n', error_buffers[i], period_size, true);
                file_helpers_write_buf(
                        "input.csv", '\n', noise_buffers[i], period_size, true);
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
