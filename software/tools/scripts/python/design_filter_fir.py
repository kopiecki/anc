#!/usr/bin/python3

from numpy import absolute, pi
from scipy.signal import kaiserord, firwin, freqz
import pylab


def design_filter(width: float, ripple_db: float, cutoff_hz: float, nyquist_hz: float) -> tuple[list[float], int]:
    order, beta = kaiserord(ripple_db, width)
    impulse_response: list[float] = firwin(
        order, cutoff_hz / nyquist_hz, window=('kaiser', beta))

    return impulse_response, order


def plot_filter(impulse_response: list[float], order: int, nyquist_hz: float) -> None:
    pylab.figure(1)
    pylab.plot(impulse_response, 'bo-', linewidth=2)
    pylab.title('Filter Coefficients (%d taps)' % order)
    pylab.grid(True)

    pylab.figure(2)
    pylab.clf()
    w, h = freqz(impulse_response)
    pylab.semilogy((w/pi)*nyquist_hz, absolute(h), linewidth=2)
    pylab.xlabel('Frequency (Hz)')
    pylab.ylabel('Gain (dB)')
    pylab.title('Frequency Response')
    pylab.grid(True)

    pylab.show()


def export_impulse_response(impulse_response: list[float], filename: str = "imp_res.csv") -> None:
    with open(filename, 'w') as file:
        for coeff in impulse_response:
            file.write(str(coeff) + '\n')


if __name__ == "__main__":
    sample_rate: float = 500
    nyquist_hz: float = sample_rate / 2
    width: float = 25 / nyquist_hz
    cutoff_hz: float = 50
    stopband_atten_db: float = 20.0

    impulse_response, order = design_filter(
        width, stopband_atten_db, cutoff_hz, nyquist_hz)
    plot_filter(impulse_response, order, nyquist_hz)
    export_impulse_response(impulse_response)
