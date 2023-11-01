/**
 * @file    anc.c
 * @brief   ANC algorithms implementation
 */

#include "anc.h"

#include "dsp/fir.h"
#include "dsp/lms.h"
#include "dsp/nlms.h"
#include "dsp/prng.h"
#include "signal/signal.h"
#include "sound_card/sound_card.h"
#include "utils/circ_fifo.h"
#include "utils/file_helpers.h"

#define BPS 2

static bool anc_init = false;

static inline uint16_t to_u16(float sample)
{
        return (sample * INT16_MAX) + UINT16_MAX / 2;
}

static inline float normalize(uint16_t sample)
{
        return 2 * ((float)sample / (float)(UINT16_MAX)) - 1;
}

static void anc_prepare_areas(
        struct alsa_resources* const alsa, unsigned char** const samples, int* steps)
{
        for (int chn = 0; chn < alsa->channels; chn++) {
                if ((alsa->areas[chn].first % 8) != 0)
                        exit(EXIT_FAILURE);

                samples[chn] =
                        (((unsigned char*)alsa->areas[chn].addr)
                         + (alsa->areas[chn].first / 8));

                if ((alsa->areas[chn].step % 16) != 0)
                        exit(EXIT_FAILURE);

                steps[chn] = alsa->areas[chn].step / 8;
                samples[chn] += steps[chn];
        }
}

void anc_loop(
        struct anc_resources* const anc, struct alsa_resources* const alsa, int r)
{
        unsigned char* samples[alsa->channels];
        int steps[alsa->channels];

        anc_prepare_areas(alsa, samples, steps);

        uint16_t* r_samples = (uint16_t*)alsa->buffer;
        for (int i = 0; i < r; i += anc->dec_cnt) {
                const uint16_t e = r_samples[i * alsa->channels + 0] << 4;
                const uint16_t x = r_samples[i * alsa->channels + 1] << 4;

                const float e_n = normalize(e);
                const float x_n = normalize(x);

                anc->error_buffer[*anc->dump_cnt] = e_n;
                anc->x_buffer[*anc->dump_cnt]     = x_n;

                circ_fifo_write(anc->x_fifo, x_n);

                float y = dsp_fir(
                        anc->x_fifo, anc->primary_w, anc->primary_order, 0);
                anc->y_buffer[*(anc->dump_cnt)++] = y;

                const float fx = dsp_fir(
                        anc->x_fifo, anc->secondary_w, anc->secondary_order, 0);
                circ_fifo_write(anc->fx_fifo, fx);

                if (anc_init) {
                        dsp_nlms(
                                anc->primary_w,
                                e_n,
                                anc->step,
                                anc->fx_fifo,
                                anc->primary_order,
                                0);
                        while (anc->dec_cnt--) {
                                const uint16_t y_dac = to_u16(y);
                                for (int i = 0; i < BPS; i++)
                                        *(samples[0] + i) =
                                                (y_dac >> i * 8) & 0xff;
                                samples[0] += steps[0];
                        }
                }
        }

        anc_init = true;
}

void anc_ident(
        struct anc_resources* const anc, struct alsa_resources* const alsa, int r)
{
        unsigned char* samples[alsa->channels];
        int steps[alsa->channels];

        anc_prepare_areas(alsa, samples, steps);

        uint16_t* r_samples = (uint16_t*)alsa->buffer;
        for (int i = 0; i < r; i += anc->dec_cnt) {
                if (anc_init) {
                        const uint16_t e = r_samples[i * alsa->channels + 0] << 4;
                        const uint16_t x = r_samples[i * alsa->channels + 1] << 4;

                        anc->error_buffer[*anc->dump_cnt] = normalize(e);
                        anc->x_buffer[*anc->dump_cnt]     = normalize(x);
                }

                uint16_t y                            = prng_xorshift();
                anc->noise_buffer[*(anc->dump_cnt)++] = normalize(y);

                while (anc->dec_cnt--) {
                        for (int j = 0; j < BPS; j++)
                                *(samples[0] + j) = (y >> j * 8) & 0xff;
                        samples[0] += steps[0];
                        y = prng_xorshift();
                }
        }

        anc_init = true;
}

void anc_dump(struct anc_resources* const anc)
{
        file_helpers_write_buf(
                "anc.txt", '\n', anc->error_buffer, *anc->dump_cnt, false);
        file_helpers_write_buf(
                "noise.txt", '\n', anc->noise_buffer, *anc->dump_cnt, false);
        file_helpers_write_buf(
                "y.txt", '\n', anc->y_buffer, *anc->dump_cnt, false);
        file_helpers_write_buf(
                "x.txt", '\n', anc->x_buffer, *anc->dump_cnt, false);
}
