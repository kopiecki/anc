/**
 * @file    main.c
 * @brief   ANC RT thread variation.
 */

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dsp/fir.h"
#include "dsp/lms.h"
#include "signal/signal.h"
#include "sound_card/sound_card.h"
#include "utils/circ_fifo.h"
#include "utils/file_helpers.h"
#include "utils/logger.h"

#define F_HZ         8000u
#define NUM_CHANNELS 1u

#define P_DEV "hw:0,0"
#define C_DEV "hw:0,0"

#define PERIOD_US 8500
#define BUFF_US   2 * PERIOD_US
#define S_SIZE    sizeof(uint32_t)

#define LMS_STEP 1.0

enum playback_buffers {
        ANTINOISE_BUFF = 0,
        NOISE_BUFF     = 1,
};

enum capture_buffers {
        ERROR_BUFF = 0,
        REF_BUFF   = 1,
};

static sound_card_t *p_dev, *c_dev;
static uint32_t *p_buff, *c_buff, *ref_buff, *error_buff;
static size_t period_size;

#define FIFO_SIZE 8192u
static struct circ_fifo x_fifo, fx_fifo;

#define PRIMARY_ORDER 512u
static float primary_w[PRIMARY_ORDER];

#define SECONDARY_ORDER 256u
static float secondary_w[SECONDARY_ORDER];

static struct signal white_noise = {
        .amplitude          = UINT16_MAX,
        .frequency          = 0.0,
        .sampling_frequency = F_HZ,
        .type               = SIGNAL_TYPE_NOISE,
        .phase              = 0,
};

static inline uint16_t convert_adc(uint32_t sample)
{
        return (sample << 4u) >> 8u;
}

static inline float normalize(uint16_t sample)
{
        return (float)sample / (float)(UINT16_MAX);
}

static inline uint16_t to_u16(float sample)
{
        return (sample + 1) * INT16_MAX;
}

static int create_capture_dev(void)
{
        const struct sound_card_config capture_device_cfg = {
                .access         = SOUND_CARD_ACCESS_INTERLEAVED,
                .channels       = NUM_CHANNELS,
                .format         = SOUND_CARD_FORMAT_U24_LE,
                .name           = C_DEV,
                .rate           = F_HZ,
                .stream         = SOUND_CARD_STREAM_CAPTURE,
                .buffer_time_us = BUFF_US,
                .period_time_us = PERIOD_US,
                .poll           = false,
                .blocking       = false,
        };

        c_dev = sound_card_construct(&capture_device_cfg);
        if (!c_dev)
                return -ENOTSUP;

        return 0;
}

static int create_playback_dev(void)
{
        const struct sound_card_config playback_device_cfg = {
                .access         = SOUND_CARD_ACCESS_INTERLEAVED,
                .channels       = NUM_CHANNELS,
                .format         = SOUND_CARD_FORMAT_U24_LE,
                .name           = P_DEV,
                .rate           = F_HZ,
                .stream         = SOUND_CARD_STREAM_PLAYBACK,
                .buffer_time_us = BUFF_US,
                .period_time_us = PERIOD_US,
                .poll           = false,
                .blocking       = false,
        };

        p_dev = sound_card_construct(&playback_device_cfg);
        if (!p_dev)
                return -ENOTSUP;

        return 0;
}

static int create_devs(void)
{
        int error = create_playback_dev();
        if (error < 0)
                return error;

        error = create_capture_dev();
        if (error < 0)
                return error;

        return error;
}

static int create_buffers(void)
{
        period_size = sound_card_get_period_size_bytes(p_dev) / S_SIZE;

        p_buff = calloc(period_size * NUM_CHANNELS, S_SIZE);
        if (!p_buff)
                return -ENOMEM;

        c_buff = calloc(period_size * NUM_CHANNELS, S_SIZE);
        if (!c_buff)
                return -ENOMEM;

        ref_buff = calloc(F_HZ * 8, S_SIZE);
        if (!ref_buff)
                return -ENOMEM;

        error_buff = calloc(F_HZ * 8, S_SIZE);
        if (!error_buff)
                return -ENOMEM;

        int error = circ_fifo_construct(&x_fifo, NULL, FIFO_SIZE);
        if (error < 0)
                return error;

        error = circ_fifo_construct(&fx_fifo, NULL, FIFO_SIZE);
        if (error < 0)
                return error;

        LOG_INFO("Created buffers");

        return error;
}

[[maybe_unused]] static int set_scheduler(
        int policy, int priority, const char* policy_name)
{
        if (!priority) {
                priority = sched_get_priority_max(policy);
                if (priority < 0) {
                        LOG_ERROR("Cannot get maximum priority");
                        return priority;
                }
        }

        struct sched_param param;
        param.sched_priority = priority;

        int error = sched_setscheduler(0, policy, &param);
        if (error < 0) {
                LOG_ERROR("Cannot set scheduler policy %s", policy_name);
                return error;
        }

        LOG_INFO(
                "Scheduler policy set to %s with priority %d",
                policy_name,
                priority);

        return error;
}

static int create_rt_thread(
        int policy,
        const char* policy_name,
        int priority,
        pthread_t* thread,
        void* (*worker)(void*))
{
        struct sched_param param;
        pthread_attr_t attr;

        int error = pthread_attr_init(&attr);
        if (error < 0) {
                LOG_ERROR(
                        "Cannot initialize pthread attribute [%d, %s]",
                        error,
                        strerror(error));
                return error;
        }

        error = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        if (error < 0) {
                LOG_ERROR(
                        "Cannot set inherited scheduler [%d, %s]",
                        error,
                        strerror(error));
        }

        error = pthread_attr_setschedpolicy(&attr, policy);
        if (error < 0) {
                LOG_ERROR(
                        "Cannot set scheduler policy %s [%d, %s]",
                        policy_name,
                        error,
                        strerror(error));
        }

        if (!priority) {
                priority = sched_get_priority_max(policy);
                if (priority < 0) {
                        LOG_ERROR("Cannot get maximum priority");
                        return priority;
                }
        }

        param.sched_priority = priority;
        error                = pthread_attr_setschedparam(&attr, &param);
        if (error < 0) {
                LOG_ERROR(
                        "Cannot set thread scheduler [%d, %s]",
                        error,
                        strerror(error));
                return error;
        }

        error = pthread_create(thread, &attr, worker, NULL);
        if (error < 0) {
                LOG_ERROR("Cannot create thread [%d, %s]", error, strerror(error));
                return error;
        }

        return error;
}

[[maybe_unused]] static void* identificate(void* foo)
{
        LOG_INFO("Started identification loop");

        size_t i = 0;
        while (i < (F_HZ * 4)) {

                for (size_t j = 0u; j < period_size; j++) {
                        const uint16_t noise_sample =
                                signal_generate(&white_noise, NULL, 0);

                        p_buff[j + ANTINOISE_BUFF] = noise_sample;
                        ref_buff[j + i]            = noise_sample;
                }

                int error = sound_card_playback(p_dev, p_buff, period_size);
                if (error < 0)
                        return NULL;

                error = sound_card_capture(c_dev, c_buff, period_size);
                if (error < 0)
                        return NULL;

                for (size_t j = 0; j < period_size; j++) {
                        error_buff[j + i] = convert_adc(c_buff[j]);
                }

                LOG_INFO("Captured full buffer, i = %d", i);

                i += period_size;
        }

        file_helpers_write_buf(
                "error.txt", '\n', (float*)error_buff, F_HZ * 4, false);
        LOG_INFO("Written error to error.txt");

        file_helpers_write_buf(
                "noise.txt", '\n', (float*)ref_buff, F_HZ * 4, false);
        LOG_INFO("Written identification signal to noise.txt");

        LOG_INFO("Identification loop finished");

        return NULL;
}

static void fxlms(void)
{
        size_t k = 0;
        for (size_t j = 0; j < period_size; j += (NUM_CHANNELS / 2)) {
                const uint16_t x = convert_adc(c_buff[j + REF_BUFF]);
                circ_fifo_write(&x_fifo, normalize(x));

                float y = dsp_fir(&x_fifo, primary_w, PRIMARY_ORDER, k);
                p_buff[j + ANTINOISE_BUFF] = to_u16(y);

                const float fx =
                        dsp_fir(&x_fifo, secondary_w, SECONDARY_ORDER, k);
                circ_fifo_write(&fx_fifo, fx);

                const uint16_t e = convert_adc(c_buff[j + ERROR_BUFF]);
                dsp_lms(primary_w,
                        normalize(e),
                        LMS_STEP,
                        &fx_fifo,
                        PRIMARY_ORDER,
                        k++);
        }
}

static void* anc_loop(void* foo)
{
        LOG_INFO("Started ANC loop");

        size_t i = 0;
        while (i < (F_HZ * 4)) {

                for (size_t j = 0u; j < period_size; j++) {
                        const uint16_t noise_sample =
                                signal_generate(&white_noise, NULL, 0);

                        p_buff[j + NOISE_BUFF] = noise_sample;
                }

                int error = sound_card_playback(p_dev, p_buff, period_size);
                if (error < 0)
                        return NULL;

                error = sound_card_capture(c_dev, c_buff, period_size);
                if (error < 0)
                        return NULL;

                fxlms();

                LOG_INFO("Captured full buffer, i = %d", i);

                i += period_size;
        }

        file_helpers_write_buf(
                "error.txt", '\n', (float*)error_buff, F_HZ * 4, false);
        LOG_INFO("Written error to error.txt");

        file_helpers_write_buf(
                "noise.txt", '\n', (float*)ref_buff, F_HZ * 4, false);
        LOG_INFO("Written identification signal to noise.txt");

        LOG_INFO("Identification loop finished");

        return NULL;
}

void cleanup(void)
{
        sound_card_destruct(c_dev);
        sound_card_destruct(p_dev);

        free(c_buff);
        free(p_buff);
        free(ref_buff);
        free(error_buff);

        circ_fifo_destruct(&x_fifo);
        circ_fifo_destruct(&fx_fifo);

        LOG_INFO("Resources deallocated");
}

int main(int argc, char* argv[])
{
        (void)argc;
        (void)argv;

        pthread_t anc_thread;

        int error = create_devs();
        if (error < 0)
                return error;

        error = create_buffers();
        if (error < 0)
                return error;

        error = create_rt_thread(SCHED_RR, "SCHED_RR", 0, &anc_thread, anc_loop);
        if (error < 0)
                return error;

        error = pthread_join(anc_thread, NULL);
        if (error < 0)
                return error;

        cleanup();

        return 0;
}
