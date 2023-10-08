#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import welch
from scipy.io import wavfile

def compute_and_plot_psd(data, fs, seg=2**10):
    f, p_den = welch(data, fs=fs, window='hann', nperseg=seg)

    plt.plot(f[1:-2], 10*np.log10(p_den)[1:-2], linewidth=4, label="PSD")
    plt.xlabel('f, Hz')
    plt.xticks([0, 0.5, 1])
    plt.yticks([82, 84, 86, 88, 90])

    plt.ylabel('PSD')
    plt.grid()

    return f, p_den

if __name__ == "__main__":
    font = {'size'   : 34}
    plt.rc('font', **font)

    fs, data = wavfile.read('500_hz.wav')
    data = data - np.mean(data)
    data = [ (ele / 2**16.) * 2 - 1 for ele in data]

    compute_and_plot_psd(data, fs)
    plt.axvline(x = 500, color = 'r', linestyle = '-', label="500 Hz")
    plt.legend()
    plt.show()
