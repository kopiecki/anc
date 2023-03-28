/**
 * @file    sound_card.h
 * @brief   Sound card ADT. In fact, this is just asoundlib fascade with
 * addidional logging to make programming with ALSA easier and more
 * transparent.
 */

#ifndef SOUND_CARD_H_
#define SOUND_CARD_H_

#include <stddef.h>
#include <stdint.h>

#define SOUND_CARD_MAX_CHANNEL	 2u
#define SOUND_CARD_FORMAT_UINT16 2
#define SOUND_CARD_FORMAT_FLOAT	 14

#ifndef SOUND_CARD_FORMAT
#define SOUND_CARD_FORMAT SOUND_CARD_FORMAT_FLOAT
#define SAMPLE_FORMAT	  float
#endif

/* Opaque sound card handle */
typedef struct sound_card_s sound_card_t;

enum sound_card_stream
{
	SOUND_CARD_STREAM_PLAYBACK = 0,
	SOUND_CARD_STREAM_CAPTURE,
	SOUND_CARD_STREAM_FULL_DUPLEX,
};

struct sound_card_config
{
	const char* name;

	enum sound_card_stream stream;

	uint32_t rate;
	uint8_t channels;

	uint32_t buffer_time_us;
	uint32_t period_time_us;
};

struct sound_card_playback_task
{
	SAMPLE_FORMAT* sample_buffers[SOUND_CARD_MAX_CHANNEL];
	size_t size;
	uint16_t num_of_periods;
};

sound_card_t* const sound_card_init(
	const struct sound_card_config* const config);
void sound_card_deinit(sound_card_t* const this);

size_t sound_card_get_period_size_bytes(sound_card_t* const this);

int sound_card_capture(sound_card_t* const this, void* buffer, size_t size);
int sound_card_playback(
	sound_card_t* const this,
	struct sound_card_playback_task* const task);

#endif /* SOUND_CARD_H_ */
