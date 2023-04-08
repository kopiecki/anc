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
            self._order = len(self._coeffs)

        logger.info("Created FIR filter with order {}", self._order)

    def __call__(self, x_ring: deque) -> np.ndarray:
        if len(x_ring) != self._order:
            x_ring = list(islice(x_ring, 0, self._order))
        return self._coeffs @ x_ring

    def plot(self):
        plt.figure(1)
        self._plot_coeffs()
        plt.figure(2)
        self._plot__freq_res()
        plt.show()

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
        if len(new_coeffs) is self._order:
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


if __name__ == "__main__":
    filter = FIR(sampling_hz=1000, stopband_ripple_db=30,
                 trans_band_width_hz=100, cutoff_hz=250)
    filter.plot()
    filter.export()
