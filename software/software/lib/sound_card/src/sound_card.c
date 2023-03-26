/**
 * @file    sound_card.c
 * @brief   Sound card driver ADT.
 */

#define ALSA_PCM_NEW_HW_PARAMS_API

#include "sound_card/sound_card.h"

#include <alsa/asoundlib.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "logger/logger.h"
#include "utils/assert.h"

#define SINE_FREQUENCY 100
#define MAX_PHASE      2.0f * M_PI

#define FORMAT_SIZE_BITS(format)  snd_pcm_format_physical_width(format)
#define FORMAT_SIZE_BYTES(format) (snd_pcm_format_physical_width(format) >> 3u)

struct sound_card_s
{
        const char* name;

        uint8_t channels;
        uint32_t rate;

        snd_pcm_t* pcm_handle;
        snd_pcm_hw_params_t* pcm_hw_params_handle;
        snd_pcm_sw_params_t* pcm_sw_params_handle;

        snd_pcm_uframes_t buffer_size_frames;
        snd_pcm_uframes_t period_size_frames;

        uint8_t frame_size_bytes;
        snd_pcm_format_t format;
};

static inline const snd_pcm_uframes_t sound_card_get_start_threshold(
        sound_card_t* const self)
{
        return (self->buffer_size_frames / self->period_size_frames)
               * self->period_size_frames;
}

static int sound_card_open(
        sound_card_t* const self,
        const struct sound_card_config* const config)
{
        ASSERT(self != NULL);
        ASSERT(config != NULL);

        int error =
                snd_pcm_open(&self->pcm_handle, self->name, config->stream, 0);
        if (error < 0)
                LOG_ERROR(
                        "Cannot open PCM device %s [%d, %s]",
                        self->name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "PCM device %s with %s stream opened successfully",
                        self->name,
                        snd_pcm_stream_name(config->stream));

        return error;
}

static int sound_card_allocate_hw_params(sound_card_t* const self)
{
        ASSERT(self != NULL);

        int error = snd_pcm_hw_params_malloc(&self->pcm_hw_params_handle);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot allocate parameters for PCM device %s [%s]",
                        self->name,
                        snd_strerror(error));
                return error;
        }

        error = snd_pcm_hw_params_any(
                self->pcm_handle, self->pcm_hw_params_handle);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot get hardware parameters for PCM device %s [%s]",
                        self->name,
                        snd_strerror(error));
                snd_pcm_hw_params_free(self->pcm_hw_params_handle);
        }
        else
                LOG_DEBUG("Allocated parameters for PCM device %s", self->name);

        return error;
}

static int sound_card_allocate_sw_params(sound_card_t* const self)
{
        ASSERT(self != NULL);

        int error = snd_pcm_sw_params_malloc(&self->pcm_sw_params_handle);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot allocate software parameters for PCM device %s "
                        "[%s]",
                        self->name,
                        snd_strerror(error));
                return error;
        }

        error = snd_pcm_sw_params_current(
                self->pcm_handle, self->pcm_sw_params_handle);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot get software parameters for PCM device %s [%s]",
                        self->name,
                        snd_strerror(error));
                snd_pcm_sw_params_free(self->pcm_sw_params_handle);
        }
        else
                LOG_DEBUG(
                        "Allocated software parameters for PCM device %s",
                        self->name);

        return error;
}


static void sound_card_deallocate(sound_card_t* const self)
{
        ASSERT(self != NULL);

        if (self->pcm_hw_params_handle != NULL)
                snd_pcm_hw_params_free(self->pcm_hw_params_handle);
        if (self->pcm_sw_params_handle != NULL)
                snd_pcm_sw_params_free(self->pcm_sw_params_handle);

        free(self);

        LOG_DEBUG("Sound card resources deallocated");
}

static int sound_card_hw_set_access(sound_card_t* const self)
{
        ASSERT(self != NULL);

        int error = snd_pcm_hw_params_set_access(
                self->pcm_handle,
                self->pcm_hw_params_handle,
                SND_PCM_ACCESS_RW_INTERLEAVED);
        if (error < 0)
                LOG_ERROR(
                        "Cannot set PCM device %s access [%s]",
                        self->name,
                        snd_strerror(error));
        else
                LOG_DEBUG("Setted access for PCM device %s", self->name);

        return error;
}

static int sound_card_hw_set_format(
        sound_card_t* const self,
        enum sound_card_sample_format format)
{
        ASSERT(self != NULL);

        int error = snd_pcm_hw_params_set_format(
                self->pcm_handle, self->pcm_hw_params_handle, format);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot set PCM device %s format [%s]",
                        self->name,
                        snd_strerror(error));
                return error;
        }
        else
                LOG_DEBUG(
                        "Setted PCM device %s format %s",
                        self->name,
                        snd_pcm_format_name(format));

        self->format = format;

        return error;
}

static int sound_card_hw_set_rate(sound_card_t* const self, uint32_t rate)
{
        ASSERT(self != NULL);

        uint32_t exact_rate = rate;
        int error           = snd_pcm_hw_params_set_rate_near(
                self->pcm_handle, self->pcm_hw_params_handle, &exact_rate, NULL);
        if (error < 0)
        {
                LOG_ERROR(
                        "Can't set rate %d for PCM device %s [%s]",
                        rate,
                        self->name,
                        snd_strerror(error));
                return error;
        }
        else if (exact_rate != rate)
        {
                LOG_WARNING(
                        "The rate %d Hz is not supported by hardware.\n ==> "
                        "Using "
                        "%d "
                        "Hz instead",
                        rate,
                        exact_rate);
        }
        else
                LOG_DEBUG(
                        "Setted rate %d for PCM device %s", exact_rate, self->name);

        self->rate = rate;

        return error;
}

static int sound_card_hw_set_channels(sound_card_t* const self, uint8_t channels)
{
        ASSERT(self != NULL);

        int error = snd_pcm_hw_params_set_channels(
                self->pcm_handle, self->pcm_hw_params_handle, channels);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot set PCM device %s channels [%s]",
                        self->name,
                        snd_strerror(error));

                return error;
        }
        else
                LOG_DEBUG(
                        "Setted channels %d for PCM device %s",
                        channels,
                        self->name);

        self->channels = channels;

        return error;
}

static int sound_card_hw_set_buffer(sound_card_t* const self, uint32_t buffer_time_us)
{
        ASSERT(self != NULL);

        uint32_t exact_buffer_time_us = buffer_time_us;
        int error                     = snd_pcm_hw_params_set_buffer_time_near(
                self->pcm_handle,
                self->pcm_hw_params_handle,
                &exact_buffer_time_us,
                NULL);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot set PCM device %s buffer time %d [%s]",
                        self->name,
                        buffer_time_us,
                        snd_strerror(error));
                return error;
        }
        else if (exact_buffer_time_us != buffer_time_us)
                LOG_WARNING(
                        "The period size %d is not supported by hardware.\n "
                        "==> "
                        "Using "
                        "%d "
                        "instead",
                        buffer_time_us,
                        exact_buffer_time_us);
        else
                LOG_DEBUG(
                        "Setted PCM device %s buffer time %d",
                        self->name,
                        buffer_time_us);

        error = snd_pcm_hw_params_get_buffer_size(
                self->pcm_hw_params_handle, &(self->buffer_size_frames));
        if (error < 0)
                LOG_ERROR(
                        "Cannot get PCM device %s buffer size in frames [%d, "
                        "%s]",
                        self->name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "PCM device %s configured with buffer size %d frames",
                        self->name,
                        self->buffer_size_frames);

        return error;
}

static int sound_card_hw_set_period(sound_card_t* const self, uint32_t period_time_us)
{
        ASSERT(self != NULL);

        uint32_t exact_period_time_us = period_time_us;
        int error                     = snd_pcm_hw_params_set_period_time_near(
                self->pcm_handle,
                self->pcm_hw_params_handle,
                &exact_period_time_us,
                NULL);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot set PCM device %s period time %d [%s]",
                        self->name,
                        period_time_us,
                        snd_strerror(error));
                return error;
        }
        else if (exact_period_time_us != period_time_us)
                LOG_WARNING(
                        "The period time %d is not supported by hardware.\n "
                        "==> "
                        "Using "
                        "%d "
                        "instead",
                        period_time_us,
                        exact_period_time_us);
        else
                LOG_DEBUG(
                        "Setted PCM device %s period size %d",
                        self->name,
                        period_time_us);

        error = snd_pcm_hw_params_get_period_size(
                self->pcm_hw_params_handle, &(self->period_size_frames), NULL);
        if (error < 0)
                LOG_ERROR(
                        "Cannot get PCM device %s period size in frames [%d, "
                        "%s]",
                        self->name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "PCM device %s configured with period size %d frames",
                        self->name,
                        self->period_size_frames);

        return error;
}

static int sound_card_hw_write_params(sound_card_t* const self)
{
        ASSERT(self != NULL);

        int error =
                snd_pcm_hw_params(self->pcm_handle, self->pcm_hw_params_handle);
        if (error < 0)
                LOG_ERROR(
                        "Cannot save hardware parameters for PCM device %s "
                        "parameters [%s]",
                        self->name,
                        snd_strerror(error));
        else
                LOG_DEBUG("Saved PCM device %s hardware parameters", self->name);

        return error;
}

static int sound_card_sw_set_start_threshold(sound_card_t* const self)
{
        ASSERT(self != NULL);

        snd_pcm_uframes_t start_threshold = sound_card_get_start_threshold(self);
        int error = snd_pcm_sw_params_set_start_threshold(
                self->pcm_handle, self->pcm_sw_params_handle, start_threshold);
        if (error < 0)
                LOG_ERROR(
                        "Cannot set start threshold %d for playback for PCM "
                        "device %s [%s]",
                        self->name,
                        start_threshold,
                        snd_strerror(error));
        else
                LOG_DEBUG(
                        "Setted start threshold %d for PCM device %s",
                        start_threshold,
                        self->name);

        return error;
}

static int sound_card_sw_set_min_transfer(sound_card_t* const self)
{
        ASSERT(self != NULL);

        int error = snd_pcm_sw_params_set_avail_min(
                self->pcm_handle,
                self->pcm_sw_params_handle,
                self->period_size_frames);
        if (error < 0)
                LOG_ERROR(
                        "Cannot set minimum transfer %d for PCM device %s [%s]",
                        self->period_size_frames,
                        self->name,
                        snd_strerror(error));
        else
                LOG_DEBUG(
                        "Setted %d minimum transfer for PCM device %s",
                        self->period_size_frames,
                        self->name);

        return error;
}

static int sound_card_sw_write_params(sound_card_t* const self)
{
        ASSERT(self != NULL);

        int error =
                snd_pcm_sw_params(self->pcm_handle, self->pcm_sw_params_handle);
        if (error < 0)
                LOG_ERROR(
                        "Cannot save software parameters for PCM device %s "
                        "[%s]",
                        self->name,
                        snd_strerror(error));
        else
                LOG_DEBUG("Saved PCM device %s software parameters", self->name);

        return error;
}

static int sound_card_hw_configure(
        sound_card_t* const self,
        const struct sound_card_config* const config)
{
        ASSERT(self != NULL);

        int error = sound_card_hw_set_access(self);
        if (error < 0)
                return error;

        error = sound_card_hw_set_format(self, config->format);
        if (error < 0)
                return error;

        error = sound_card_hw_set_channels(self, config->channels);
        if (error < 0)
                return error;

        error = sound_card_hw_set_rate(self, config->rate);
        if (error < 0)
                return error;

        error = sound_card_hw_set_buffer(self, config->buffer_time_us);
        if (error < 0)
                return error;

        error = sound_card_hw_set_period(self, config->period_time_us);
        if (error < 0)
                return error;

        error = sound_card_hw_write_params(self);
        if (error < 0)
                return error;

        return error;
}

static int sound_card_sw_configure(sound_card_t* const self)
{
        ASSERT(self != NULL);

        int error = sound_card_sw_set_start_threshold(self);
        if (error < 0)
                return error;

        error = sound_card_sw_set_min_transfer(self);
        if (error < 0)
                return error;

        return error;
}

static int sound_card_xrun_recovery(sound_card_t* const self, int error)
{
        if (error == -EPIPE)
        {
                LOG_WARNING("Underrun occured for PCM device %s", self->name);

                error = snd_pcm_prepare(self->pcm_handle);
                if (error < 0)
                {
                        LOG_ERROR(
                                "Can't recovery from underrun [%s]",
                                snd_strerror(error));
                        return error;
                }
        }
        else if (error == -ESTRPIPE)
        {
                while ((error = snd_pcm_resume(self->pcm_handle)) == -EAGAIN)
                        sleep(1);
                if (error < 0)
                {
                        error = snd_pcm_prepare(self->pcm_handle);
                        if (error < 0)
                        {
                                LOG_ERROR(
                                        "Can't recover from suspend [%s]",
                                        snd_strerror(error));
                                return error;
                        }
                }
        }

        return error;
}

sound_card_t* const sound_card_init(const struct sound_card_config* const config)
{
        ASSERT(config != NULL);
        ASSERT(config->name != NULL);

        sound_card_t* self = malloc(sizeof(*self));
        if (self == NULL)
                return NULL;

        self->name = config->name;

        int error = sound_card_open(self, config);
        if (error < 0)
                goto fail;

        error = sound_card_allocate_hw_params(self);
        if (error < 0)
                goto fail;

        error = sound_card_hw_configure(self, config);
        if (error < 0)
                goto fail;

        error = sound_card_allocate_sw_params(self);
        if (error < 0)
                goto fail;

        error = sound_card_sw_configure(self);
        if (error < 0)
                goto fail;

        LOG_INFO("PCM device %s configured", self->name);

        return self;

fail:
        sound_card_deallocate(self);
        return NULL;
}

void sound_card_deinit(sound_card_t* const self)
{
        ASSERT(self != NULL);

        snd_pcm_drain(self->pcm_handle);
        snd_pcm_close(self->pcm_handle);

        sound_card_deallocate(self);
}

size_t sound_card_get_period_size_bytes(sound_card_t* const self)
{
        ASSERT(self != NULL);

        const size_t frame_size_bytes =
                FORMAT_SIZE_BYTES(self->format) * self->channels;
        return frame_size_bytes * self->period_size_frames;
}

int sound_card_capture(sound_card_t* self, void* buffer, size_t size)
{
        ASSERT(self != NULL);
}

static void sound_card_prepare_buffer(
        sound_card_t* const self,
        struct sound_card_playback_task* const task,
        uint16_t* const buffer)
{
        size_t j = 0u;
        for (size_t i = 0u; i < task->size; i++)
                for (uint8_t channel = 0u; channel < self->channels; channel++)
                        buffer[j++] = task->sample_buffers[channel][i];
}

int sound_card_playback(
        sound_card_t* const self,
        struct sound_card_playback_task* const task)
{
        ASSERT(self != NULL);
        ASSERT(task != NULL);

        uint16_t* buffer = malloc(
                sound_card_get_period_size_bytes(self) * task->num_of_periods);

        sound_card_prepare_buffer(self, task, buffer);

        uint16_t num_of_periods = task->num_of_periods;
        while (num_of_periods--)
        {
                uint16_t* ptr = buffer;
                int cptr      = self->period_size_frames;
                while (cptr > 0)
                {
                        int error = snd_pcm_writei(self->pcm_handle, ptr, cptr);
                        if (error == -EAGAIN)
                                continue;
                        if (error < 0)
                        {
                                if (sound_card_xrun_recovery(self, error) < 0)
                                        return error;
                                break;
                        }
                        ptr += error * self->channels;
                        cptr -= error;
                }
        }

        return 0;
}
