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
        sound_card_t* const this)
{
        return (this->buffer_size_frames / this->period_size_frames)
               * this->period_size_frames;
}

static void generate_sine(
        sound_card_t* const this,
        const snd_pcm_channel_area_t* areas,
        snd_pcm_uframes_t offset,
        double* _phase_1,
        double* _phase_2,
        double frequency)
{
        double phase_1 = *_phase_1;
        double phase_2 = *_phase_2;

        double step_1 = MAX_PHASE * SINE_FREQUENCY / (double)this->rate;
        double step_2 = MAX_PHASE * SINE_FREQUENCY * 3 / ((double)this->rate * 2);

        uint8_t* samples[this->channels];
        int steps[this->channels];

        for (uint8_t channel = 0u; channel < this->channels; channel++)
        {
                /* samples byte address + channel offset */
                samples[channel] =
                        (((uint8_t*)areas[channel].addr)
                         + (areas[channel].first / 8));

                /* step in bytes */
                steps[channel] = areas[channel].step / 8;

                /* += 0 */
                samples[channel] += offset * steps[channel];
        }

        int count = this->period_size_frames;
        while (count-- > 0)
        {
                int res_1 = (sin(phase_1) + sin(phase_2)) * INT16_MAX;
                for (uint8_t channel = 0; channel < this->channels; channel++)
                {
                        for (int i = 0; i < FORMAT_SIZE_BYTES(this->format); i++)
                        {
                                if (channel == 0)
                                {
                                        *(samples[channel] + i) =
                                                (res_1 >> (i * 8)) & 0xFF;
                                }
                                else
                                        *(samples[channel] + i) = 0;
                        }
                        samples[channel] += steps[channel];
                }

                phase_1 += step_1;
                phase_2 += step_2;

                if (phase_1 >= MAX_PHASE)
                        phase_1 += MAX_PHASE;
                if (phase_2 >= MAX_PHASE)
                        phase_2 += MAX_PHASE;
        }
        *_phase_1 = phase_1;
        *_phase_2 = phase_2;
}

static int sound_card_open(
        sound_card_t* const this,
        const struct sound_card_config* const config)
{
        ASSERT(this != NULL);
        ASSERT(config != NULL);

        int error =
                snd_pcm_open(&this->pcm_handle, this->name, config->stream, 0);
        if (error < 0)
                LOG_ERROR(
                        "Cannot open PCM device %s [%d, %s]",
                        this->name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "PCM device %s with %s stream opened successfully",
                        this->name,
                        snd_pcm_stream_name(config->stream));

        return error;
}

static int sound_card_allocate_hw_params(sound_card_t* const this)
{
        ASSERT(this != NULL);

        int error = snd_pcm_hw_params_malloc(&this->pcm_hw_params_handle);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot allocate parameters for PCM device %s [%s]",
                        this->name,
                        snd_strerror(error));
                return error;
        }

        error = snd_pcm_hw_params_any(
                this->pcm_handle, this->pcm_hw_params_handle);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot get hardware parameters for PCM device %s [%s]",
                        this->name,
                        snd_strerror(error));
                snd_pcm_hw_params_free(this->pcm_hw_params_handle);
        }
        else
                LOG_DEBUG("Allocated parameters for PCM device %s", this->name);

        return error;
}

static int sound_card_allocate_sw_params(sound_card_t* const this)
{
        ASSERT(this != NULL);

        int error = snd_pcm_sw_params_malloc(&this->pcm_sw_params_handle);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot allocate software parameters for PCM device %s "
                        "[%s]",
                        this->name,
                        snd_strerror(error));
                return error;
        }

        error = snd_pcm_sw_params_current(
                this->pcm_handle, this->pcm_sw_params_handle);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot get software parameters for PCM device %s [%s]",
                        this->name,
                        snd_strerror(error));
                snd_pcm_sw_params_free(this->pcm_sw_params_handle);
        }
        else
                LOG_DEBUG(
                        "Allocated software parameters for PCM device %s",
                        this->name);

        return error;
}


static void sound_card_deallocate(sound_card_t* const this)
{
        ASSERT(this != NULL);

        if (this->pcm_hw_params_handle != NULL)
                snd_pcm_hw_params_free(this->pcm_hw_params_handle);
        if (this->pcm_sw_params_handle != NULL)
                snd_pcm_sw_params_free(this->pcm_sw_params_handle);

        free(this);

        LOG_DEBUG("Sound card resources deallocated");
}

static int sound_card_hw_set_access(sound_card_t* const this)
{
        ASSERT(this != NULL);

        int error = snd_pcm_hw_params_set_access(
                this->pcm_handle,
                this->pcm_hw_params_handle,
                SND_PCM_ACCESS_RW_INTERLEAVED);
        if (error < 0)
                LOG_ERROR(
                        "Cannot set PCM device %s access [%s]",
                        this->name,
                        snd_strerror(error));
        else
                LOG_DEBUG("Setted access for PCM device %s", this->name);

        return error;
}

static int sound_card_hw_set_format(
        sound_card_t* const this,
        enum sound_card_sample_format format)
{
        ASSERT(this != NULL);

        int error = snd_pcm_hw_params_set_format(
                this->pcm_handle, this->pcm_hw_params_handle, SND_PCM_FORMAT_S16);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot set PCM device %s format [%s]",
                        this->name,
                        snd_strerror(error));
                return error;
        }
        else
                LOG_DEBUG(
                        "Setted PCM device %s format %s",
                        this->name,
                        snd_pcm_format_name(SND_PCM_FORMAT_S16));

        this->format = format;

        return error;
}

static int sound_card_hw_set_rate(sound_card_t* const this, uint32_t rate)
{
        ASSERT(this != NULL);

        uint32_t exact_rate = rate;
        int error           = snd_pcm_hw_params_set_rate_near(
                this->pcm_handle, this->pcm_hw_params_handle, &exact_rate, NULL);
        if (error < 0)
        {
                LOG_ERROR(
                        "Can't set rate %d for PCM device %s [%s]",
                        rate,
                        this->name,
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
                        "Setted rate %d for PCM device %s", exact_rate, this->name);

        this->rate = rate;

        return error;
}

static int sound_card_hw_set_channels(sound_card_t* const this, uint8_t channels)
{
        ASSERT(this != NULL);

        int error = snd_pcm_hw_params_set_channels(
                this->pcm_handle, this->pcm_hw_params_handle, channels);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot set PCM device %s channels [%s]",
                        this->name,
                        snd_strerror(error));

                return error;
        }
        else
                LOG_DEBUG(
                        "Setted channels %d for PCM device %s",
                        channels,
                        this->name);

        this->channels = channels;

        return error;
}

static int sound_card_hw_set_buffer(sound_card_t* const this, uint32_t buffer_time_us)
{
        ASSERT(this != NULL);

        uint32_t exact_buffer_time_us = buffer_time_us;
        int error                     = snd_pcm_hw_params_set_buffer_time_near(
                this->pcm_handle,
                this->pcm_hw_params_handle,
                &exact_buffer_time_us,
                NULL);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot set PCM device %s buffer time %d [%s]",
                        this->name,
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
                        this->name,
                        buffer_time_us);

        error = snd_pcm_hw_params_get_buffer_size(
                this->pcm_hw_params_handle, &(this->buffer_size_frames));
        if (error < 0)
                LOG_ERROR(
                        "Cannot get PCM device %s buffer size in frames [%d, "
                        "%s]",
                        this->name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "PCM device %s configured with buffer size %d frames",
                        this->name,
                        this->buffer_size_frames);

        return error;
}

static int sound_card_hw_set_period(sound_card_t* const this, uint32_t period_time_us)
{
        ASSERT(this != NULL);

        uint32_t exact_period_time_us = period_time_us;
        int error                     = snd_pcm_hw_params_set_period_time_near(
                this->pcm_handle,
                this->pcm_hw_params_handle,
                &exact_period_time_us,
                NULL);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot set PCM device %s period time %d [%s]",
                        this->name,
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
                        this->name,
                        period_time_us);

        error = snd_pcm_hw_params_get_period_size(
                this->pcm_hw_params_handle, &(this->period_size_frames), NULL);
        if (error < 0)
                LOG_ERROR(
                        "Cannot get PCM device %s period size in frames [%d, "
                        "%s]",
                        this->name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "PCM device %s configured with period size %d frames",
                        this->name,
                        this->period_size_frames);

        return error;
}

static int sound_card_hw_write_params(sound_card_t* const this)
{
        ASSERT(this != NULL);

        int error =
                snd_pcm_hw_params(this->pcm_handle, this->pcm_hw_params_handle);
        if (error < 0)
                LOG_ERROR(
                        "Cannot save hardware parameters for PCM device %s "
                        "parameters [%s]",
                        this->name,
                        snd_strerror(error));
        else
                LOG_DEBUG("Saved PCM device %s hardware parameters", this->name);

        return error;
}

static int sound_card_sw_set_start_threshold(sound_card_t* const this)
{
        ASSERT(this != NULL);

        snd_pcm_uframes_t start_threshold = sound_card_get_start_threshold(this);
        int error = snd_pcm_sw_params_set_start_threshold(
                this->pcm_handle, this->pcm_sw_params_handle, start_threshold);
        if (error < 0)
                LOG_ERROR(
                        "Cannot set start threshold %d for playback for PCM "
                        "device %s [%s]",
                        this->name,
                        start_threshold,
                        snd_strerror(error));
        else
                LOG_DEBUG(
                        "Setted start threshold %d for PCM device %s",
                        start_threshold,
                        this->name);

        return error;
}

static int sound_card_sw_set_min_transfer(sound_card_t* const this)
{
        ASSERT(this != NULL);

        int error = snd_pcm_sw_params_set_avail_min(
                this->pcm_handle,
                this->pcm_sw_params_handle,
                this->period_size_frames);
        if (error < 0)
                LOG_ERROR(
                        "Cannot set minimum transfer %d for PCM device %s [%s]",
                        this->period_size_frames,
                        this->name,
                        snd_strerror(error));
        else
                LOG_DEBUG(
                        "Setted %d minimum transfer for PCM device %s",
                        this->period_size_frames,
                        this->name);

        return error;
}

static int sound_card_sw_write_params(sound_card_t* const this)
{
        ASSERT(this != NULL);

        int error =
                snd_pcm_sw_params(this->pcm_handle, this->pcm_sw_params_handle);
        if (error < 0)
                LOG_ERROR(
                        "Cannot save software parameters for PCM device %s "
                        "[%s]",
                        this->name,
                        snd_strerror(error));
        else
                LOG_DEBUG("Saved PCM device %s software parameters", this->name);

        return error;
}

static int sound_card_hw_configure(
        sound_card_t* const this,
        const struct sound_card_config* const config)
{
        ASSERT(this != NULL);

        int error = sound_card_hw_set_access(this);
        if (error < 0)
                return error;

        error = sound_card_hw_set_format(this, config->format);
        if (error < 0)
                return error;

        error = sound_card_hw_set_channels(this, config->channels);
        if (error < 0)
                return error;

        error = sound_card_hw_set_rate(this, config->rate);
        if (error < 0)
                return error;

        error = sound_card_hw_set_buffer(this, config->buffer_time_us);
        if (error < 0)
                return error;

        error = sound_card_hw_set_period(this, config->period_time_us);
        if (error < 0)
                return error;

        error = sound_card_hw_write_params(this);
        if (error < 0)
                return error;

        return error;
}

static int sound_card_sw_configure(sound_card_t* const this)
{
        ASSERT(this != NULL);

        int error = sound_card_sw_set_start_threshold(this);
        if (error < 0)
                return error;

        error = sound_card_sw_set_min_transfer(this);
        if (error < 0)
                return error;

        return error;
}

static int sound_card_xrun_recovery(sound_card_t* const this, int error)
{
        if (error == -EPIPE)
        {
                LOG_WARNING("Underrun occured for PCM device %s", this->name);

                error = snd_pcm_prepare(this->pcm_handle);
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
                while ((error = snd_pcm_resume(this->pcm_handle)) == -EAGAIN)
                        sleep(1);
                if (error < 0)
                {
                        error = snd_pcm_prepare(this->pcm_handle);
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

        sound_card_t* this = malloc(sizeof(*this));
        if (this == NULL)
                return NULL;

        this->name = config->name;

        int error = sound_card_open(this, config);
        if (error < 0)
                goto fail;

        error = sound_card_allocate_hw_params(this);
        if (error < 0)
                goto fail;

        error = sound_card_hw_configure(this, config);
        if (error < 0)
                goto fail;

        error = sound_card_allocate_sw_params(this);
        if (error < 0)
                goto fail;

        error = sound_card_sw_configure(this);
        if (error < 0)
                goto fail;

        LOG_INFO("PCM device %s configured", this->name);

        return this;

fail:
        sound_card_deallocate(this);
        return NULL;
}

void sound_card_deinit(sound_card_t* const this)
{
        ASSERT(this != NULL);

        snd_pcm_drain(this->pcm_handle);
        snd_pcm_close(this->pcm_handle);

        sound_card_deallocate(this);
}

int sound_card_capture(sound_card_t* this, void* buffer, size_t size)
{
        ASSERT(this != NULL);
}

static int16_t* const sound_card_allocate_playback_samples(
        sound_card_t* const this,
        uint8_t num_of_periods)
{
        const size_t frame_size_bytes =
                FORMAT_SIZE_BYTES(this->format) * this->channels;
        const size_t period_size_bytes =
                frame_size_bytes * this->period_size_frames;

        int16_t* const samples = malloc(period_size_bytes * num_of_periods);
        if (samples == NULL)
                LOG_ERROR("Cannot allocate samples");

        return samples;
}

static snd_pcm_channel_area_t* const sound_card_allocate_playback_areas(
        sound_card_t* const this)
{
        snd_pcm_channel_area_t* const areas =
                calloc(this->channels, sizeof(snd_pcm_channel_area_t));
        if (areas == NULL)
                LOG_ERROR("Cannot allocate areas");

        return areas;
}

static void sound_card_prepare_playback_areas(
        sound_card_t* const this,
        int16_t* const samples,
        snd_pcm_channel_area_t* const areas)
{
        uint8_t format_size_bits = FORMAT_SIZE_BITS(this->format);
        for (uint8_t channel = 0u; channel < this->channels; channel++)
        {
                areas[channel].addr  = samples;
                areas[channel].first = channel * format_size_bits;
                areas[channel].step  = this->channels * format_size_bits;
        }
}

int sound_card_playback(
        sound_card_t* const this,
        const struct sound_card_playback_task* const task)
{
        int16_t* const samples =
                sound_card_allocate_playback_samples(this, task->num_of_periods);
        if (samples == NULL)
                goto bad_alloc;

        snd_pcm_channel_area_t* const areas =
                sound_card_allocate_playback_areas(this);
        if (areas == NULL)
                goto bad_alloc;

        sound_card_prepare_playback_areas(this, samples, areas);

        uint16_t num_of_periods = task->num_of_periods;
        while (num_of_periods--)
        {
                double phase_1 = 0.0f;
                double phase_2 = 0.0f;
                generate_sine(this, areas, 0, &phase_1, &phase_2, 0);
                int16_t* ptr = samples;
                int cptr     = this->period_size_frames;
                while (cptr > 0)
                {
                        int error = snd_pcm_writei(this->pcm_handle, ptr, cptr);
                        if (error == -EAGAIN)
                                continue;
                        if (error < 0)
                        {
                                if (sound_card_xrun_recovery(this, error) < 0)
                                        return error;
                                break;
                        }
                        ptr += error * this->channels;
                        cptr -= error;
                }
        }

        return 0;

bad_alloc:
        if (samples != NULL)
                free(samples);
        if (areas != NULL)
                free(areas);
        return -ENOMEM;
}
