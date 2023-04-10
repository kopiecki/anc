#!/usr/bin/python3

import numpy as np
import scipy.signal as scisig
import matplotlib.pyplot as plt

from fir import FIR
from loguru import logger

if __name__ == "__main__":
    SAMPLING_HZ = 4000
    NUM_OF_SAMPLES = 2 * SAMPLING_HZ
    model = FIR(sampling_hz=SAMPLING_HZ, stopband_ripple_db=30,
                       cutoff_hz=200, trans_band_width_hz=1000)

    x = np.random.normal(0, 1, NUM_OF_SAMPLES)
    y = model.lib_filter(x)

    print(len(x))
    print(len(y))

    c = scisig.correlate(y, x)
    xx = scisig.correlate(x, x)

    gk = c / np.max(xx)

    est = gk[NUM_OF_SAMPLES - 1: NUM_OF_SAMPLES - 1 + len(model.coeffs)]

    logger.info("Model coeffs = {}", model.coeffs)
    logger.info("Model est = {}", est)

    model.plot(freqs=False) 

    plt.figure(2)
    plt.plot(est)
    plt.grid()
    plt.show()
