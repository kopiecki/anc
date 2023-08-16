/**
 * @file    sound_card.h
 * @brief   Sound card ADT. In fact, this is just asoundlib fascade with
 * addidional logging to make programming with ALSA easier and more
 * transparent.
 */

#include "sound_card/sound_card.h"

#include <alsa/asoundlib.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "utils/assert.h"
#include "utils/logger.h"

#define FORMAT_SIZE_BITS(format)  snd_pcm_format_physical_width(format)
#define FORMAT_SIZE_BYTES(format) (snd_pcm_format_physical_width(format) >> 3u)

#define PERIOD_SIZE_FRAMES(period_size_bytes, channels)                        \
        (period_size_bytes / (channels * sizeof(float)))

struct sound_card_s {
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
        snd_pcm_access_t access;
        snd_pcm_stream_t stream;

        bool poll;
        struct pollfd* poll_ufds;
        int poll_count;
};

#if 0
static inline void sound_card_prepare_buffer_interleaved(
        sound_card_t* const self,
        struct sound_card_task* const task,
        void* const buffer)
{
        size_t j = 0u;
        for (size_t i = 0u; i < task->size; i++)
                for (uint8_t channel = 0u; channel < self->channels; channel++)
                        buffer[j++] = task->sample_buffers[channel][i];
}
#endif

static inline snd_pcm_uframes_t sound_card_get_start_threshold(
        sound_card_t* const self)
{
        return (self->buffer_size_frames / self->period_size_frames)
               * self->period_size_frames;
}

static int sound_card_open(
        sound_card_t* const self, const struct sound_card_config* const config)
{
        ASSERT(self != NULL);
        ASSERT(config != NULL);

        int error = snd_pcm_open(
                &self->pcm_handle, self->name, (snd_pcm_stream_t)config->stream, 0);
        if (error < 0)
                LOG_ERROR(
                        "Cannot open PCM device %s [%d, %s]",
                        self->name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "PCM device %s with %s stream opened "
                        "successfully",
                        self->name,
                        snd_pcm_stream_name((snd_pcm_stream_t)config->stream));

        self->stream = (snd_pcm_stream_t)config->stream;

        return error;
}

static int sound_card_allocate_hw_params(sound_card_t* const self)
{
        ASSERT(self != NULL);

        int error = snd_pcm_hw_params_malloc(&self->pcm_hw_params_handle);
        if (error < 0) {
                LOG_ERROR(
                        "Cannot allocate parameters for PCM device %s "
                        "[%s]",
                        self->name,
                        snd_strerror(error));
                return error;
        }

        error = snd_pcm_hw_params_any(
                self->pcm_handle, self->pcm_hw_params_handle);
        if (error < 0) {
                LOG_ERROR(
                        "Cannot get hardware parameters for PCM device "
                        "%s [%s]",
                        self->name,
                        snd_strerror(error));
                snd_pcm_hw_params_free(self->pcm_hw_params_handle);
        } else
                LOG_DEBUG("Allocated parameters for PCM device %s", self->name);

        return error;
}

static int sound_card_allocate_sw_params(sound_card_t* const self)
{
        ASSERT(self != NULL);

        int error = snd_pcm_sw_params_malloc(&self->pcm_sw_params_handle);
        if (error < 0) {
                LOG_ERROR(
                        "Cannot allocate software parameters for PCM "
                        "device %s "
                        "[%s]",
                        self->name,
                        snd_strerror(error));
                return error;
        }

        error = snd_pcm_sw_params_current(
                self->pcm_handle, self->pcm_sw_params_handle);
        if (error < 0) {
                LOG_ERROR(
                        "Cannot get software parameters for PCM device "
                        "%s [%s]",
                        self->name,
                        snd_strerror(error));
                snd_pcm_sw_params_free(self->pcm_sw_params_handle);
        } else
                LOG_DEBUG(
                        "Allocated software parameters for PCM device "
                        "%s",
                        self->name);

        return error;
}

static void sound_card_deallocate(sound_card_t* const self)
{
        if (self != NULL) {
                snd_pcm_drain(self->pcm_handle);

                if (self->pcm_hw_params_handle != NULL)
                        snd_pcm_hw_params_free(self->pcm_hw_params_handle);
                if (self->pcm_sw_params_handle != NULL)
                        snd_pcm_sw_params_free(self->pcm_sw_params_handle);
                if (self->poll)
                        free(self->poll_ufds);

                snd_pcm_close(self->pcm_handle);

                free(self);
        }

        LOG_DEBUG("Sound card resources deallocated");
}

static int sound_card_hw_set_access(
        sound_card_t* const self, enum sound_card_access access)
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
                LOG_DEBUG(
                        "Setted access %s for PCM device %s",
                        snd_pcm_access_name((snd_pcm_access_t)access),
                        self->name);

        self->access = (snd_pcm_access_t)access;

        return error;
}

static int sound_card_hw_set_format(sound_card_t* const self, int format)
{
        ASSERT(self != NULL);

        int error = snd_pcm_hw_params_set_format(
                self->pcm_handle, self->pcm_hw_params_handle, format);
        if (error < 0) {
                LOG_ERROR(
                        "Cannot set PCM device %s format [%s]",
                        self->name,
                        snd_strerror(error));
                return error;
        } else
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
        if (error < 0) {
                LOG_ERROR(
                        "Can't set rate %d for PCM device %s [%s]",
                        rate,
                        self->name,
                        snd_strerror(error));
                return error;
        } else if (exact_rate != rate) {
                LOG_WARNING(
                        "The rate %d Hz is not supported by "
                        "hardware.\n ==> "
                        "Using "
                        "%d "
                        "Hz instead",
                        rate,
                        exact_rate);
        } else
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
        if (error < 0) {
                LOG_ERROR(
                        "Cannot set PCM device %s channels [%s]",
                        self->name,
                        snd_strerror(error));

                return error;
        } else
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
        if (error < 0) {
                LOG_ERROR(
                        "Cannot set PCM device %s buffer time %d [%s]",
                        self->name,
                        buffer_time_us,
                        snd_strerror(error));
                return error;
        } else if (exact_buffer_time_us != buffer_time_us)
                LOG_WARNING(
                        "The period size %d is not supported by "
                        "hardware.\n "
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
                        "Cannot get PCM device %s buffer size in "
                        "frames [%d, "
                        "%s]",
                        self->name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "PCM device %s configured with buffer size %d "
                        "frames",
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
        if (error < 0) {
                LOG_ERROR(
                        "Cannot set PCM device %s period time %d [%s]",
                        self->name,
                        period_time_us,
                        snd_strerror(error));
                return error;
        } else if (exact_period_time_us != period_time_us)
                LOG_WARNING(
                        "The period time %d is not supported by "
                        "hardware.\n "
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
                        "Cannot get PCM device %s period size in "
                        "frames [%d, "
                        "%s]",
                        self->name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "PCM device %s configured with period size %d "
                        "frames",
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
                        "Cannot save hardware parameters for PCM "
                        "device %s "
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
                        "Cannot set start threshold %d for playback "
                        "for PCM "
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
                        "Cannot set minimum transfer %d for PCM device "
                        "%s [%s]",
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
                        "Cannot save software parameters for PCM "
                        "device %s "
                        "[%s]",
                        self->name,
                        snd_strerror(error));
        else
                LOG_DEBUG("Saved PCM device %s software parameters", self->name);

        return error;
}

static int sound_card_hw_configure(
        sound_card_t* const self, const struct sound_card_config* const config)
{
        ASSERT(self != NULL);

        int error = sound_card_hw_set_access(self, config->access);
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

        error = sound_card_sw_write_params(self);
        if (error < 0)
                return error;

        return error;
}

static int sound_card_configure_poll(sound_card_t* const self)
{
        if (!self->poll)
                return 0;

        self->poll_count = snd_pcm_poll_descriptors_count(self->pcm_handle);
        if (self->poll_count <= 0) {
                LOG_ERROR(
                        "Invalid poll descriptors count [%d] for PCM device %s",
                        self->poll_count,
                        self->name);
                return self->poll_count;
        }

        self->poll_ufds =
                malloc(sizeof(struct pollfd) * (size_t)self->poll_count);
        if (self->poll_ufds == NULL) {
                LOG_ERROR("Cannot allocate memory for ufds struct");
                return -ENOMEM;
        }

        int error = snd_pcm_poll_descriptors(
                self->pcm_handle, self->poll_ufds, (unsigned int)self->poll_count);
        if (error < 0) {
                LOG_ERROR(
                        "Cannot obtain poll descriptors for PCM device %s [%s]",
                        self->name,
                        snd_strerror(error));
                return error;
        }

        if (self->stream == (snd_pcm_stream_t)SOUND_CARD_STREAM_CAPTURE)
                error = snd_pcm_start(self->pcm_handle);
        if (self->stream == (snd_pcm_stream_t)SOUND_CARD_STREAM_PLAYBACK)
                error = snd_pcm_prepare(self->pcm_handle);

        return error;
}

sound_card_t* sound_card_construct(const struct sound_card_config* const config)
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

        self->poll = config->poll;
        error      = sound_card_configure_poll(self);
        if (error < 0)
                goto fail;

        LOG_INFO("PCM device %s configured", self->name);

        return self;

fail:
        sound_card_deallocate(self);
        return NULL;
}

void sound_card_destruct(sound_card_t* const self)
{
        ASSERT(self != NULL);
        sound_card_deallocate(self);
}

int sound_card_get_period_size_bytes(sound_card_t* const self)
{
        ASSERT(self != NULL);

        const int frame_size_bytes =
                FORMAT_SIZE_BYTES(self->format) * self->channels;
        return frame_size_bytes * (int)self->period_size_frames;
}

static int sound_card_epipe_recovery(sound_card_t* const self)
{
        LOG_WARNING(
                "Underrun occured for PCM device %s %s",
                self->name,
                snd_pcm_stream_name(self->stream));

        int error = snd_pcm_prepare(self->pcm_handle);
        if (error < 0) {
                LOG_ERROR(
                        "Can't recovery from underrun [%s]", snd_strerror(error));
                return error;
        }

        return error;
}

static int sound_card_estrpipe_recovery(sound_card_t* const self)
{
        int error = snd_pcm_resume(self->pcm_handle);
        do {
                error = snd_pcm_resume(self->pcm_handle);
        } while (error == -EAGAIN);

        if (error < 0) {
                error = snd_pcm_prepare(self->pcm_handle);
                if (error < 0) {
                        LOG_ERROR(
                                "Can't recover from suspend "
                                "[%s]",
                                snd_strerror(error));
                        return error;
                }
        }

        return error;
}

static int sound_card_xrun_recovery(sound_card_t* const self, int error)
{
        switch (error) {
                case -EPIPE:
                        error = sound_card_epipe_recovery(self);
                        break;
                case -ESTRPIPE:
                        error = sound_card_estrpipe_recovery(self);
                        break;
                default:
                        break;
        }

        return error;
}

int sound_card_playback(sound_card_t* const self, void* buffer, size_t size)
{
        ASSERT(self != NULL);
        ASSERT(buffer != NULL);

        int error = 0;
        const snd_pcm_sframes_t sframes =
                snd_pcm_writei(self->pcm_handle, buffer, size);
        if (sframes < 0) {
                LOG_WARNING("Write error, trying recover...");
                error = sound_card_xrun_recovery(self, error);
                if (error < 0) {
                        LOG_ERROR(
                                "Write error for PCM device %s [%s]",
                                self->name,
                                snd_strerror(error));
                        return error;
                }
        }

        return error;
}

int sound_card_synchronize(sound_card_t* const playback, sound_card_t* const capture)
{
        int error = snd_pcm_link(playback->pcm_handle, capture->pcm_handle);
        if (error < 0)
                LOG_WARNING(
                        "Cannot link PCM devices %s and %s [%s]",
                        playback->name,
                        capture->name,
                        snd_strerror(error));

        error = snd_pcm_start(capture->pcm_handle);
        if (error < 0)
                LOG_WARNING("Cannot start capture device");

        return error;
}

int sound_card_capture(sound_card_t* const self, void* buffer, size_t size)
{
        ASSERT(self != NULL);
        ASSERT(buffer != NULL);

        int error = 0;
        const snd_pcm_sframes_t rframes =
                snd_pcm_readi(self->pcm_handle, buffer, size);
        if (rframes < 0) {
                LOG_WARNING("Read error, trying recover...");
                error = sound_card_xrun_recovery(self, error);
                if (error < 0) {
                        LOG_ERROR(
                                "Read error for PCM device %s [%s]",
                                self->name,
                                snd_strerror(error));
                        return error;
                }
        }

        return error;
}

static int sound_card_wait_for_poll(sound_card_t* const self)
{
        uint16_t revents = 0;
        while (1) {
                poll(self->poll_ufds, (nfds_t)self->poll_count, -1);
                snd_pcm_poll_descriptors_revents(
                        self->pcm_handle,
                        self->poll_ufds,
                        (uint8_t)self->poll_count,
                        &revents);

                if (revents & POLLERR)
                        return -EIO;
                else if ((revents & POLLOUT) && (self->stream == (snd_pcm_stream_t)SOUND_CARD_STREAM_PLAYBACK))
                        return 0;
                else if ((revents & POLLIN) && (self->stream == (snd_pcm_stream_t)SOUND_CARD_STREAM_CAPTURE))
                        return 0;
                else if ((revents & POLLIN) && (revents & POLLOUT))
                        return 0;
        }
}

int sound_card_poll_is_ready(sound_card_t* const self, bool* const ready)
{
        int error = sound_card_wait_for_poll(self);
        if (error < 0) {
                snd_pcm_state_t state = snd_pcm_state(self->pcm_handle);
                if ((state == SND_PCM_STATE_XRUN)
                    || (state == SND_PCM_STATE_SUSPENDED)) {
                        error = (state == SND_PCM_STATE_XRUN) ? -EPIPE :
                                                                -ESTRPIPE;
                        error = sound_card_xrun_recovery(self, error);
                        if (error < 0)
                                return error;
                } else {
                        LOG_ERROR(
                                "Wait for poll failed for PCM device "
                                "%s",
                                self->name);
                        return error;
                }
        }

        *ready = true;
        return 0;
}
