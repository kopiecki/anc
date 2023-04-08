#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt

from loguru import logger
from collections import deque
from fir import FIR


class LMS(object):
    """
    Calculate weights of FIR filter with least mean squares algorithm:

    w(n + 1) = w(n) + step * x(n) * [d(n) - w^T(n)x(n)]

    where:
        - w(n) - filter weights at "time" n
        - x(n) - input sample at "time" n
        - d(n) - object output sample at "time" n
        - step - algorithm step
    """

    def __init__(self,
                 sampling_hz: int,
                 step: float,
                 initial_w: np.ndarray):

        self._step = step
        self._filter = FIR(sampling_hz=sampling_hz, initial_w=initial_w)
        self._w_history = np.array([self._filter.coeffs])

    def __call__(self, x_ring: deque, error: float) -> FIR:
        self._filter.coeffs = self._filter.coeffs + \
            [self._step * x * error for x in x_ring]
        self._w_history = np.vstack([self._w_history, self._filter.coeffs])
        
        return self._filter

    @property 
    def filter(self) -> FIR:
        return self._filter

    def plot_w_history(self):
        self._w_history = np.matrix.transpose(self._w_history)
        for i, (filter_coeffs, lms_filter_coeffs) in enumerate(zip(self._filter.coeffs, self._w_history)):
            plt.plot(lms_filter_coeffs, label="lms coeff {}".format(i))
            plt.plot(np.ones(self._w_history.shape[1]) * filter_coeffs,
                     label="filter coeff {}".format(i))
        plt.grid()
        plt.legend()
        plt.show()


if __name__ == "__main__":
    SAMPLING_HZ = 1000

    filter = FIR(sampling_hz=1000, stopband_ripple_db=30,
                 trans_band_width_hz=250, cutoff_hz=250)

    noise = np.random.normal(0, 1, 1000)
    lms = LMS(1000, 0.05, np.zeros(filter.order))
    
    x_ring = deque([0] * filter.order, maxlen=filter.order)

    lms_filter = lms.filter
    for sample in noise:
        x_ring.appendleft(sample)

        d = filter(x_ring)
        y = lms_filter(x_ring)

        error = d - y
        lms_filter = lms(x_ring, error)

    lms.plot_w_history()
