/**
 * @file    anc.h
 * @brief   Encapsulated ANC collection algorithms header.
 */

#ifndef ANC_H_
#define ANC_H_

#include <stdint.h>

#include "sound_card/sound_card.h"
#include "utils/circ_fifo.h"

/**
 * @brief Encapsulated ANC resources.
 */
struct anc_resources {
        float* const x_buffer;
        float* const y_buffer;
        float* const error_buffer;
        float* const noise_buffer;
        float* const primary_w;
        float* const secondary_w;
        struct circ_fifo* const x_fifo;
        struct circ_fifo* const fx_fifo;
        size_t* dump_cnt;
        unsigned primary_order;
        unsigned secondary_order;
        unsigned dec_cnt;
        unsigned step;
};

/**
 * @brief Encapsulated ALSA resources.
 */
struct alsa_resources {
        snd_pcm_channel_area_t* const areas;
        char* const buffer;
        unsigned channels;
};

/**
 * @brief Perform ANC process.
 *
 * @param anc   ANC resources
 * @param alsa  ALSA resources
 * @param r     Number of received samples in bytes
 */
void anc_loop(
        struct anc_resources* const anc, struct alsa_resources* const alsa, int r);

/**
 * @brief Perform identfication process.
 *
 * @param anc    ANC resources
 * @param alsa   ALSA resources
 * @param r      Number of received samples in bytes
 */
void anc_ident(
        struct anc_resources* const anc, struct alsa_resources* const alsa, int r);

/**
 * @brief Dump ANC data for later analysis.
 *
 * @param anc   ANC resources.
 */
void anc_dump(struct anc_resources* const anc);

#endif /* ANC_H_ */
