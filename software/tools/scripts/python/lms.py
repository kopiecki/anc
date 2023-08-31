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

    def plot_w_history(self, plot_filter: bool = False):
        self._w_history = np.matrix.transpose(self._w_history)
        for i, (filter_coeffs, lms_filter_coeffs) in enumerate(zip(self._filter.coeffs, self._w_history)):
            plt.plot(lms_filter_coeffs, label="lms coeff {}".format(i))

        plt.grid()
        plt.xlabel("i")
        plt.ylabel("w_l(i)")
        plt.show()


def __generate_x(sampling_hz: int, num_of_samples: int):
    t = np.arange(num_of_samples) / sampling_hz

    def omega(freq: int) -> float:
        return 2 * np.pi * freq

    sine_100_hz = 2 * np.sin(omega(100) * t)
    sine_250_hz = 0.5 * np.sin(omega(300) * t)
    noise = 0.1 * np.random.normal(0, 1, num_of_samples)

    return sine_100_hz + sine_250_hz + noise


def __estimate_sec() -> None:
    SAMPLING_HZ = 4000
    ORDER = 150

    lms = LMS(SAMPLING_HZ, 0.005, np.zeros(ORDER))

    x = np.genfromtxt("input.csv", delimiter='\n')
    y = np.genfromtxt("capture.csv", delimiter='\n')
    x = x[int(SAMPLING_HZ / 2) : int(len(x) / 4) - 200]
    y = y[int(SAMPLING_HZ / 2) + 200 : int(len(y) / 4)]

    x_ring = deque([0] * ORDER, maxlen=ORDER)

    lms_filter = lms.filter
    for _x, _d in zip(x, y):
        x_ring.appendleft(_x)

        y = lms_filter(x_ring)

        error = _d - y
        lms_filter = lms(x_ring, error)

    logger.info("Last LMS estimate = {}", lms_filter.coeffs)
    lms.plot_w_history(False)


def __estimate_filter() -> None:
    SAMPLING_HZ = 4000
    NUM_OF_SAMPLES = 2 * SAMPLING_HZ
    order = 200

    filter = FIR(sampling_hz=SAMPLING_HZ, stopband_ripple_db=30,
                 cutoff_hz=200, trans_band_width_hz=500)

    lms = LMS(NUM_OF_SAMPLES, 0.0005, np.zeros(order))
    x = np.genfromtxt("noise.txt", delimiter='\n')
    y = np.genfromtxt("error.txt", delimiter='\n')

    x_ring = deque([0] * order, maxlen=order)

    lms_filter = lms.filter
    for x_s, y_s in zip(x, y):
        x_ring.appendleft(x_s)

        y = lms_filter(x_ring)

        error = y_s - y
        lms_filter = lms(x_ring, error)

    logger.info("Last LMS estimate = {}", lms_filter.coeffs)
    logger.info("Real filter coeffs = {}", filter.coeffs)
    lms.plot_w_history()


if __name__ == "__main__":
    __estimate_filter()
