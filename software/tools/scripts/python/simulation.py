#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt

from loguru import logger
from collections import deque
from fir import FIR
from lms import LMS


class Simulation(object):
    def __init__(self,
                 sampling_hz: int,
                 num_of_samples: int,
                 primary_path: FIR,
                 secondary_path: FIR,
                 secondary_estimate: FIR,
                 lms_step: float):

        self._sampling_hz = sampling_hz
        self._num_of_samples = num_of_samples

        self._primary = primary_path
        self._secondary = secondary_path
        self._secondary_estimate = secondary_estimate

        self._x = self.__generate_x()

        self._x_ring = deque([0] * self._primary.order,
                             maxlen=self._primary.order)
        self._y_ring = deque([0] * self._primary.order,
                             maxlen=self._primary.order)
        self._filtered_x_ring = deque([0] * self._primary.order,
                                      maxlen=self._primary.order)

        self._lms = LMS(self._sampling_hz, lms_step,
                        np.zeros(self._primary.order))

    def __call__(self, fxlms: bool = True):
        primary_output = np.array([])
        errors = np.array([])

        lms_filter = self._lms.filter
        for sample in self._x:
            self._x_ring.appendleft(sample)

            d = self._primary(self._x_ring)
            primary_output = np.append(primary_output, d)

            y = lms_filter(self._x_ring)
            self._y_ring.appendleft(y)

            y_p = self._secondary(self._y_ring)

            if fxlms:
                x_p = self._secondary_estimate(self._x_ring)
                self._filtered_x_ring.appendleft(x_p)

            error = d - y_p
            errors = np.append(errors, error)

            lms_filter = self._lms(
                self._filtered_x_ring if fxlms else self._x_ring, error)

        return primary_output, errors

    @property
    def lms(self) -> LMS:
        return self._lms

    def __generate_x(self):
        t = np.arange(self._num_of_samples) / self._sampling_hz

        def omega(freq: int) -> float:
            return 2 * np.pi * freq

        sine_100_hz = 2 * np.sin(omega(100) * t)
        sine_250_hz = 0.5 * np.sin(omega(300) * t)
        noise = 0.1 * np.random.normal(0, 1, self._num_of_samples)

        return sine_100_hz + sine_250_hz + noise


if __name__ == "__main__":
    SAMPLING_HZ = 4000
    NUM_OF_SAMPLES = 2 * SAMPLING_HZ

    primary_path = FIR(sampling_hz=SAMPLING_HZ, stopband_ripple_db=30,
                       cutoff_hz=300, trans_band_width_hz=500)
    secondary_path = FIR(sampling_hz=SAMPLING_HZ, stopband_ripple_db=20,
                         cutoff_hz=500, trans_band_width_hz=300)
    secondary_path_estimate = FIR(sampling_hz=SAMPLING_HZ, stopband_ripple_db=20,
                                  cutoff_hz=500, trans_band_width_hz=300)

    sim = Simulation(SAMPLING_HZ, NUM_OF_SAMPLES, primary_path,
                     secondary_path, secondary_path_estimate, 0.005)

    primary_output, errors = sim(False)

    plt.figure(1)
    plt.plot(primary_output)
    plt.plot(errors)

    plt.figure(2)
    sim.lms.plot_w_history()
