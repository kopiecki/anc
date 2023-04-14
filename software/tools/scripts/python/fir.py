#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as scisig

from collections import deque
from itertools import islice
from loguru import logger


class FIR(object):
    """
    Design FIR filter or make a placeholder for adaptive calculation.
    """

    def __init__(self,
                 sampling_hz: int,
                 stopband_ripple_db: float = None,
                 cutoff_hz: int = None,
                 trans_band_width_hz: int = None,
                 initial_w: np.ndarray = None):

        self._sampling_hz = sampling_hz
        self._nyquist_hz = self._sampling_hz / 2

        if stopband_ripple_db and cutoff_hz and trans_band_width_hz is not None:
            self._design(stopband_ripple_db, cutoff_hz, trans_band_width_hz)
        else:
            self._coeffs = initial_w
            self._order = len(initial_w)

        logger.info("Created FIR filter with order {}", self._order)

    def __call__(self, x_ring: deque) -> np.ndarray:
        if len(x_ring) != self._order:
            x_ring = list(islice(x_ring, 0, self._order))
        return self._coeffs @ x_ring

    def lib_filter(self, x):
        return scisig.lfilter(self._coeffs, 1.0, x)

    def plot(self, freqs: bool = True, imp: bool = True):

        i = 1
        if imp:
            plt.figure(i)
            i = i + 1
            self._plot_coeffs()
        
        if freqs:
            plt.figure(i)
            self._plot__freq_res()

    def export(self, file: str = None):
        if file is None:
            file = "fir_coeffs_{}.csv".format(self._order)
        with open(file, 'w') as f:
            for coeff in self._coeffs:
                f.write(str(coeff) + '\n')

    @property
    def order(self):
        return self._order

    @property
    def coeffs(self):
        return self._coeffs

    @property
    def x_ring(self):
        return self._x_ring

    @coeffs.setter
    def coeffs(self, new_coeffs: np.ndarray) -> np.ndarray:
        if len(new_coeffs) == self._order:
            self._coeffs = new_coeffs
        else:
            logger.error("Cannot update coeffs because order doesn't match!")

    def _plot_coeffs(self):
        plt.plot(self._coeffs, 'bo-', linewidth=2)
        plt.title('Filter coefficients (order = %d)' % self._order)
        plt.grid()

    def _plot__freq_res(self):
        w, h = scisig.freqz(self._coeffs)
        x = (w / np.pi) * self._nyquist_hz
        y = np.absolute(h)

        plt.semilogy(x, y, linewidth=2)
        plt.xlabel('Frequency (Hz)')
        plt.ylabel('Gain (dB)')
        plt.title('Frequency Response')
        plt.grid()

    def _design(self, stopband_atten_db: float, cutoff_hz: float, trans_band_width_hz: float) -> None:
        self._order, beta = scisig.kaiserord(
            stopband_atten_db, trans_band_width_hz / self._nyquist_hz)
        self._coeffs = scisig.firwin(self._order, cutoff_hz /
                                     self._nyquist_hz, window=('kaiser', beta))


def __generate_x(sampling_hz, num_of_samples):
        t = np.arange(num_of_samples) / sampling_hz

        def omega(freq: int) -> float:
            return 2 * np.pi * freq

        sine_100_hz = 1 * np.sin(omega(5) * t)
        sine_250_hz = 0.2 * np.sin(omega(500) * t)

        return sine_100_hz + sine_250_hz

if __name__ == "__main__":
    SAMPLING_HZ = 4000
    NUM_OF_SAMPLES = SAMPLING_HZ
    filter = FIR(sampling_hz=SAMPLING_HZ, stopband_ripple_db=30,
                       cutoff_hz=200, trans_band_width_hz=250)
    
    x = __generate_x(SAMPLING_HZ, NUM_OF_SAMPLES)

    filter.plot()
    filter.export()
    filtered = filter.lib_filter(x)

    plt.plot(x)
    plt.plot(filtered)
    plt.grid()
    plt.show()
