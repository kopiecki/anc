/**
 * @file    sound_card.c
 * @brief   <Provide some short description here>
 */

#include "sound_card/sound_card.h"

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include "logger/logger.h"
#include "utils/assert.h"

struct SoundCard_s
{
        snd_pcm_t* pcm_handle;
        snd_pcm_hw_params_t* pcm_params_handle;
        SoundCard_Config_t config;
};

static int sound_card_open(SoundCard_t* const this)
{
        ASSERT(this != NULL);

        int error = snd_pcm_open(
                &this->pcm_handle, this->config.name, this->config.stream, 0);
        if (error < 0)
                LOG_ERROR(
                        "Cannot open PCM device %s [%d, %s]",
                        this->config.name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "PCM device %s with %s stream opened successfully",
                        this->config.name,
                        (this->config.stream == CAPTURE) ? "capture" :
                                                           "playback");

        return error;
}

static int sound_card_allocate_params(SoundCard_t* const this)
{
        ASSERT(this != NULL);

        snd_pcm_hw_params_malloc(&this->pcm_params_handle);
        int error =
                snd_pcm_hw_params_any(this->pcm_handle, this->pcm_params_handle);
        if (error < 0)
        {
                LOG_ERROR(
                        "Cannot allocate parameters for PCM device %s [%d, %s]",
                        this->config.name,
                        error,
                        strerror(-error));
                snd_pcm_hw_params_free(this->pcm_params_handle);
        }
        else
                LOG_DEBUG(
                        "Allocated parameters for PCM device %s",
                        this->config.name);

        return error;
}

static void sound_card_deallocate(SoundCard_t* const this)
{
        ASSERT(this != NULL);

        snd_pcm_hw_params_free(this->pcm_params_handle);
        free(this);
}

static int sound_card_set_access(SoundCard_t* const this)
{
        ASSERT(this != NULL);

        int error = snd_pcm_hw_params_set_access(
                this->pcm_handle,
                this->pcm_params_handle,
                SND_PCM_ACCESS_RW_INTERLEAVED);
        if (error < 0)
                LOG_ERROR(
                        "Cannot set PCM device %s access [%d, %s]",
                        this->config.name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG("Setted access for PCM device %s", this->config.name);

        return error;
}

static int sound_card_set_format(SoundCard_t* const this)
{
        ASSERT(this != NULL);

        int error = snd_pcm_hw_params_set_format(
                this->pcm_handle, this->pcm_params_handle, this->config.format);
        if (error < 0)
                LOG_ERROR(
                        "Cannot set PCM device %s format [%d, %s]",
                        this->config.name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "Setted PCM device %s format %d",
                        this->config.name,
                        this->config.format);

        return error;
}

static int sound_card_set_rate(SoundCard_t* const this)
{
        ASSERT(this != NULL);

        uint32_t exact_rate = this->config.rate;
        int error           = snd_pcm_hw_params_set_rate_near(
                this->pcm_handle, this->pcm_params_handle, &exact_rate, NULL);
        if (error < 0)
                LOG_ERROR(
                        "Can't set rate %d for PCM device %s [%d, %s]",
                        this->config.rate,
                        this->config.name,
                        error,
                        strerror(-error));
        else if (exact_rate != this->config.rate)
        {
                LOG_WARNING(
                        "The rate %d Hz is not supported by hardware.\n ==> "
                        "Using "
                        "%d "
                        "Hz instead",
                        this->config.rate,
                        exact_rate);
                this->config.rate = exact_rate;
        }
        else
                LOG_DEBUG(
                        "Setted rate %d for PCM device %s",
                        exact_rate,
                        this->config.name);

        return error;
}

static int sound_card_set_channels(SoundCard_t* const this)
{
        ASSERT(this != NULL);

        int error = snd_pcm_hw_params_set_channels(
                this->pcm_handle, this->pcm_params_handle, this->config.channels);
        if (error < 0)
                LOG_ERROR(
                        "Cannot set PCM device %s channels [%d, %s]",
                        this->config.name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "Setted channels %d for PCM device %s",
                        this->config.channels,
                        this->config.name);

        return error;
}

static int sound_card_set_periods(SoundCard_t* const this, uint32_t periods)
{
        ASSERT(this != NULL);

        int error = snd_pcm_hw_params_set_periods(
                this->pcm_handle, this->pcm_params_handle, periods, 0);
        if (error < 0)
                LOG_ERROR(
                        "Cannot set PCM device %s periods [%d, %s]",
                        this->config.name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "Setted periods %d for PCM device %s",
                        periods,
                        this->config.name);

        return error;
}

static int sound_card_set_buffer_size(
        SoundCard_t* const this,
        size_t period_size,
        uint8_t periods,
        size_t buffer_size)
{
        ASSERT(this != NULL);

        int error = snd_pcm_hw_params_set_buffer_size(
                this->pcm_handle,
                this->pcm_params_handle,
                (period_size * buffer_size) >> 2);
        if (error < 0)
                LOG_ERROR(
                        "Cannot set PCM device %s buffer size [%d, %s]",
                        this->config.name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG(
                        "Setted PCM device %s buffer size %d",
                        this->config.name,
                        buffer_size);

        return error;
}

static int sound_card_save_params(SoundCard_t* const this)
{
        ASSERT(this != NULL);

        int error = snd_pcm_hw_params(this->pcm_handle, this->pcm_params_handle);
        if (error < 0)
                LOG_ERROR(
                        "Cannot save PCM device %s parameters [%d, %s]",
                        this->config.name,
                        error,
                        strerror(-error));
        else
                LOG_DEBUG("Saved PCM device %s parameters", this->config.name);

        return error;
}

static int sound_card_configure(SoundCard_t* const this)
{
        ASSERT(this != NULL);

        int error = sound_card_set_access(this);
        if (error < 0) return error;

        error = sound_card_set_format(this);
        if (error < 0) return error;

        error = sound_card_set_channels(this);
        if (error < 0) return error;

        error = sound_card_set_rate(this);
        if (error < 0) return error;

        error = sound_card_save_params(this);
        if (error < 0) return error;

        return error;
}

SoundCard_t* const sound_card_init(const SoundCard_Config_t* const config)
{
        ASSERT(config != NULL);
        ASSERT(config->name != NULL);

        SoundCard_t* this = malloc(sizeof(*this));
        if (this == NULL) return NULL;

        this->config = *config;

        int error = sound_card_open(this);
        if (error < 0) goto fail;

        error = sound_card_allocate_params(this);
        if (error < 0) goto fail;

        error = sound_card_configure(this);
        if (error < 0) goto fail;

        LOG_INFO("PCM device %s configured", this->config.name);

        return this;

fail:
        sound_card_deallocate(this);
        return NULL;
}

void sound_card_deinit(SoundCard_t* const this)
{
        ASSERT(this != NULL);

        if (this != NULL)
        {
                snd_pcm_drain(this->pcm_handle);
                snd_pcm_close(this->pcm_handle);
                snd_pcm_hw_params_free(this->pcm_params_handle);
                free(this);
        }
}

int sound_card_read(SoundCard_t* this, void* buffer, size_t size)
{
        ASSERT(this != NULL);
}

int sound_card_write(SoundCard_t* this, const void* buffer, size_t size)
{
        ASSERT(this != NULL);
}
