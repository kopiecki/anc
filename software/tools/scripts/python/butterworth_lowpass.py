#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
from mpmath import sqrt, power


class Butterworth_lowpass(object):
    """
    Design analog Butterworth lowpass filter in Sellen-Key topology.
    """

    def __init__(self, order: int, cutoff_hz: int):
        self._order = order
        self._cutoff_hz = cutoff_hz

        w = 2 * np.pi * cutoff_hz
        self._zeros, self._poles, self._k = signal.butter(
            order, w, analog=True, output='zpk')
        self._b, self._a = signal.zpk2tf(self._zeros, self._poles, self._k)

    def get_sk_r(self, caps: np.array) -> np.array:

        r = np.array([[]])
        for i in range(int(self._order / 2)):
            grouped = np.array(
                [self._poles[i], self._poles[len(self._poles) - i - 1]], dtype=np.complex128)

            poly = np.poly(grouped)
            poly = np.array(poly / poly[2])

            b = poly[0]
            a = poly[1]

            r = np.append(r, self.__calc_r(a, b, caps[i]))

        r = r.reshape((3, 2))

        return r

    def plot_freqs(self) -> None:

        font = {'size': 16}
        plt.rc('font', **font)

        w, h = signal.freqs(self._b, self._a)

        mag = 20 * np.log10(abs(h))
        phase = np.angle(h)

        plt.subplot(2, 1, 1)
        plt.plot(w / (2e3 * np.pi), mag, linewidth=2)
        plt.title('Charakterystyka amplitudowa')
        plt.xlabel('f, kHz')
        plt.xlim(0, 1)
        plt.xticks([0, 0.25, 0.5, 0.75, 1])
        plt.ylim(-40, 2)
        plt.ylabel('A, dB')
        plt.grid(which='both', axis='both')

        plt.subplot(2, 1, 2)
        plt.plot(w / (2e3 * np.pi), phase, linewidth=2)
        plt.title('Charakterystyka fazowa')
        plt.xlabel('f, kHz')
        plt.xlim(0, 1)
        plt.xticks([0, 0.25, 0.5, 0.75, 1])
        plt.ylim(-np.pi, np.pi)
        plt.yticks([-np.pi, -np.pi/2, 0, np.pi/2, np.pi],
                   ["$-\pi$", r"$-\dfrac{\pi}{2}$",
                    "0", r"$\dfrac{\pi}{2}$", "$\pi$"])
        plt.ylabel('$\phi$, rad')
        plt.grid(which='both', axis='both')

        plt.show()

    @staticmethod
    def __calc_r(a, b, caps):
        delta = sqrt(caps[1] * ((caps[1] * power(a, 2)) -
                     (caps[0] * 4 * b)), dtype=np.longdouble)

        r1 = (caps[1] * a - delta) / (2 * caps[0] * caps[1])
        r2 = (delta + caps[1] * a) / (2 * caps[0] * caps[1])

        return np.array([r1, r2])


if __name__ == "__main__":
    ORDER = 6
    F_HZ = 500

    C = [[560e-12, 22e-9],
         [2.7e-9, 6.8e-9],
         [4.7e-9, 5.6e-9]]

    filter = Butterworth_lowpass(ORDER, F_HZ)
    filter.plot_freqs()
    R = filter.get_sk_r(C)
