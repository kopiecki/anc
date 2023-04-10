/**
 * @file    test_sound.c
 * @brief   <Provide some short description here>
 */

#include <errno.h>
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

#define CAPTURE_FILE_LOG "capture.csv"

#define SC_PLAYBACK_NAME   "plughw:1,0"
#define SC_CAPTURE_NAME    "plughw:1,0"
#define SAMPLING_FREQUENCY 44100
#define NUM_OF_SAMPLES     SAMPLING_FREQUENCY

#define BUFFER_TIME_US 20000
#define PERIOD_TIME_US 10000
#define CHANNELS       2

static sound_card_t* __sc_playback = NULL;
static sound_card_t* __sc_capture  = NULL;

static TEST_FORMAT* __noise_buffer;
static TEST_FORMAT* __antinoise_buffer;

static TEST_FORMAT* __input_buffer;
static TEST_FORMAT* __error_buffer;

enum playback_buffers
{
        NOISE_BUFFER     = 0,
        ANTINOISE_BUFFER = 1,
};

enum capture_buffers
{
        INPUT_BUFFER = 0,
        ERROR_BUFFER = 1,
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
                .poll           = false,
        };

        const struct sound_card_config sc_capture_config = {
                .access         = SOUND_CARD_ACCESS_SEPARATE,
                .channels       = CHANNELS,
                .name           = SC_CAPTURE_NAME,
                .rate           = SAMPLING_FREQUENCY,
                .stream         = SOUND_CARD_STREAM_CAPTURE,
                .buffer_time_us = BUFFER_TIME_US,
                .period_time_us = PERIOD_TIME_US,
                .poll           = false,
        };

        __sc_playback = sound_card_construct(&sc_playback_config);
        if (__sc_playback == NULL)
                return -ENOTSUP;
        __sc_capture = sound_card_construct(&sc_capture_config);
        if (__sc_capture == NULL)
                return -ENOTSUP;

        LOG_INFO("Created sound cards");

        return 0;
}

static int __create_buffers(void)
{
        const size_t period_size_bytes_playback =
                sound_card_get_period_size_bytes(__sc_playback);
        const size_t period_size_bytes_capture =
                sound_card_get_period_size_bytes(__sc_capture);

        if (period_size_bytes_capture != period_size_bytes_playback)
        {
                LOG_ERROR("Period sized do not match!");
                return -EPERM;
        }

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

        LOG_INFO("Created buffers");

        return 0;
}

static struct sound_card_task __create_playback_task(void)
{

        const size_t period_size_bytes =
                sound_card_get_period_size_bytes(__sc_playback);

        struct sound_card_task task           = {0};
        task.sample_buffers[NOISE_BUFFER]     = __noise_buffer;
        task.sample_buffers[ANTINOISE_BUFFER] = __antinoise_buffer;
        task.size                             = period_size_bytes;

        LOG_INFO("Created playback task");

        return task;
}

static struct sound_card_task __create_capture_task(void)
{
        const size_t period_size_bytes =
                sound_card_get_period_size_bytes(__sc_capture);

        struct sound_card_task task       = {0};
        task.sample_buffers[INPUT_BUFFER] = __input_buffer;
        task.sample_buffers[ERROR_BUFFER] = __error_buffer;
        task.size                         = period_size_bytes;

        return task;
}

static void __generate_noise(TEST_FORMAT* const sample_buffer, size_t size)
{
        struct signal sine = {
                .amplitude          = 1.0f,
                .frequency          = 200,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .type               = SIGNAL_TYPE_SINE,
        };

        signal_generate(&sine, sample_buffer, size);
}

static void __generate_antinoise(TEST_FORMAT* const sample_buffer, size_t size)
{
        struct signal sine = {
                .amplitude          = -1.0f,
                .frequency          = 200,
                .sampling_frequency = SAMPLING_FREQUENCY,
                .type               = SIGNAL_TYPE_SINE,
        };

        signal_generate(&sine, sample_buffer, size);
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

int __poll_loop(
        struct sound_card_task* const playback_task,
        struct sound_card_task* const capture_task)
{
        LOG_INFO("Staring poll");

        bool ready = false;
        while (1)
        {
                int error = sound_card_poll_is_ready(__sc_playback, &ready);
                if (error < 0)
                        return error;

                if (ready)
                {
                        __generate_noise(
                                playback_task->sample_buffers[NOISE_BUFFER],
                                playback_task->size);
                        error = sound_card_playback(__sc_playback, playback_task);
                        if (error < 0)
                                ready = false;
                }
        }
}

int __write_read(
        struct sound_card_task* const playback_task,
        struct sound_card_task* const capture_task)
{
        __generate_noise(
                playback_task->sample_buffers[NOISE_BUFFER], playback_task->size);

        int error = sound_card_playback(__sc_playback, playback_task);
        if (error < 0)
                return error;

        error = sound_card_capture(__sc_capture, capture_task);
        if (error < 0)
                return error;

        sleep(1);

        return file_helpers_write_buf(
                CAPTURE_FILE_LOG,
                '\n',
                capture_task->sample_buffers[ERROR_BUFFER],
                capture_task->size / sizeof(TEST_FORMAT),
                false);
}

int __loopback(struct sound_card_task* playback_task, struct sound_card_task* capture_task)
{
        sound_card_synchronize(__sc_playback, __sc_capture);
        while (1)
        {
                sound_card_capture(__sc_capture, playback_task);
                sound_card_playback(__sc_playback, playback_task);
        }

        return file_helpers_write_buf(
                CAPTURE_FILE_LOG,
                '\n',
                capture_task->sample_buffers[ERROR_BUFFER],
                capture_task->size / sizeof(TEST_FORMAT),
                false);
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

        struct sound_card_task playback_task = __create_playback_task();
        struct sound_card_task capture_task  = __create_capture_task();

        return __loopback(&playback_task, &capture_task);
}

int main(int argc, char* argv[])
{
        LOG_INFO("Running sound test");
        int error = __test_loop();
        if (error < 0)
                LOG_ERROR("Sound test failed");
        else
                LOG_INFO("Sound test ended with success");

        return error;
}
