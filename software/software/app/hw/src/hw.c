#include "dsp/fir.h"
#include "utils/circ_fifo.h"

#define FIR_N   128u
#define FIFO_N  256u
#define I_CNT   10000000u

int main(void)
{
        float taps[128u];
        float fifo_buff[256u];

        struct circ_fifo cf;
        circ_fifo_construct(&cf, fifo_buff, FIFO_N);

        float y = 0.0f;
        for (unsigned i = 0u; i < I_CNT; i++) {
                circ_fifo_write(&cf, 1.0);
                y = dsp_fir(&cf, taps, FIR_N, 0u);
        }

        y += 1.0f;

        return 0;
}
