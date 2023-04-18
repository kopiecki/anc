/**
 * @file    test_ident.c
 * @brief   <Provide some short description here>
 */

/**
 * @file    test_anc.c
 * @brief   <Provide some short description here>
 */

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
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

#define SEC_COEFFS_FILE "sec_coeffs.csv"

#define SC_PLAYBACK_NAME   "plughw:1,0"
#define SC_CAPTURE_NAME    "plughw:1,0"
#define SAMPLING_FREQUENCY 4000
#define NUM_OF_SAMPLES     SAMPLING_FREQUENCY

#define BUFFER_TIME_US 200000
#define PERIOD_TIME_US 100000
#define CHANNELS       2
#define LMS_STEP       0.01

static size_t period_size;
bool new = false;

static sound_card_t* sc_playback = NULL;
static sound_card_t* sc_capture  = NULL;

static struct sound_card_task playback_task;
static struct sound_card_task capture_task;

static TEST_FORMAT* noise_buffer;
static TEST_FORMAT* antinoise_buffer;
static TEST_FORMAT* antinoise_buffer_tmp;

static TEST_FORMAT* input_buffer;
static TEST_FORMAT* error_buffer;

#define CIRC_FIFO_SIZE (1u << 16u)
static struct circ_fifo x_fifo;
static struct circ_fifo x_fx_fifo;
static struct circ_fifo fx_fifo;

#define SECONDARY_ORDER 100
static TEST_FORMAT secondary_coeff[SECONDARY_ORDER] = {0};

#define PRIMARY_ORDER 300
static TEST_FORMAT primary_coeff[PRIMARY_ORDER] = {0};

static struct signal sine_100_hz = {
        .amplitude          = 1.0f,
        .type               = SIGNAL_TYPE_SINE,
        .frequency          = 100.0f,
        .sampling_frequency = SAMPLING_FREQUENCY,
};

static struct signal noise = {
        .amplitude = 1.0f,
        .type      = SIGNAL_TYPE_NOISE,
};

enum playback_buffers
{
        NOISE_BUFFER     = 1,
        ANTINOISE_BUFFER = 0,
};

enum capture_buffers
{
        ERROR_BUFFER = 0,
        INPUT_BUFFER = 1,
};

static int create_cards(void)
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

        sound_card_synchronize(sc_playback, sc_capture);
        LOG_INFO("Created sound cards");

        return 0;
}

static int create_buffers(void)
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

        antinoise_buffer_tmp =
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

        int error = circ_fifo_construct(&x_fifo, NULL, CIRC_FIFO_SIZE);
        if (error < 0)
                return -EPERM;

        error = circ_fifo_construct(&fx_fifo, NULL, CIRC_FIFO_SIZE);
        if (error < 0)
                return -EPERM;

        error = circ_fifo_construct(&x_fx_fifo, NULL, CIRC_FIFO_SIZE);
        if (error < 0)
                return -EPERM;

        error = file_helpers_read_buf(SEC_COEFFS_FILE, secondary_coeff);
        if (error < 0)
                return -EPERM;

        LOG_INFO("Created buffers");

        return 0;
}

static int create_tasks(void)
{
        playback_task.sample_buffers[NOISE_BUFFER]     = noise_buffer;
        playback_task.sample_buffers[ANTINOISE_BUFFER] = antinoise_buffer;
        playback_task.size                             = period_size;

        LOG_INFO("Created playback task");

        capture_task.sample_buffers[INPUT_BUFFER] = input_buffer;
        capture_task.sample_buffers[ERROR_BUFFER] = error_buffer;
        capture_task.size                         = period_size;

        LOG_INFO("Created capture task");
}

static void cleanup(void)
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

void* playback_poll_loop(void*)
{
        LOG_INFO("Starting playback poll");

        bool ready = false;
        while (1)
        {
                int error = sound_card_poll_is_ready(sc_playback, &ready);
                if (error < 0)
                        return NULL;

                if (ready)
                {
                        signal_generate(&noise, noise_buffer, period_size);
                        error = sound_card_playback(sc_playback, &playback_task);
                        if (error < 0)
                                ready = false;
                }
        }

        LOG_INFO("Finished playback poll");
}

void* capture_poll_loop(void*)
{
        LOG_INFO("Starting capture poll");

        bool ready = false;
        while (1)
        {
                int error = sound_card_poll_is_ready(sc_capture, &ready);
                if (error < 0)
                        return NULL;

                if (ready)
                {
                        sound_card_capture(sc_capture, &capture_task);
                        if (error < 0)
                                ready = false;
                        new = true;
                }
        }

        LOG_INFO("Finished capture poll");
}

static int ident_loop(void)
{
        signal_initialize_noise();

        pthread_t capture_thread;
        pthread_t playback_thread;
        pthread_t calculation_thread;

        pthread_create(&playback_thread, NULL, playback_poll_loop, NULL);
        pthread_create(&capture_thread, NULL, capture_poll_loop, NULL);

        pthread_join(playback_thread, NULL);
        pthread_join(capture_thread, NULL);
}

static int test_ident(void)
{
        int error = create_cards();
        if (error < 0)
                return error;

        error = create_buffers();
        if (error < 0)
                return error;

        create_tasks();

        return ident_loop();
}

int main(int argc, char* argv[])
{
        LOG_INFO("Running ANC test");
        int error = test_ident();
        if (error < 0)
                LOG_ERROR("Identification test failed");
        else
                LOG_INFO("Identification test ended with success");

        return error;
}
