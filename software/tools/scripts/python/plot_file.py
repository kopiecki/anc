#!/usr/bin/python3

import argparse as arg
import numpy as np
import matplotlib.pyplot as plt

def parse_arguments() -> arg.Namespace:
    parser: arg.ArgumentParser = arg.ArgumentParser()
    parser.add_argument("--files", type=str, nargs='*')
    parser.add_argument("--fs", type=float, nargs=1)
    return parser.parse_args()


def read_file(file_names: list[str]) -> list[list[float]]:
    data: list[list[float]] = []
    for file_name in file_names:
        with open(file_name) as file:
            file_data: list[float] = []
            for line in file.readlines():
                sample = float(line)
                file_data.append(sample)
        data.append(file_data)
    return data


def plot(buffers: list[float], names: list[str], sample_rate: int) -> None:
    ORDER = 18
    N = len(buffers[0])
    t = np.arange(N) / sample_rate

    delay = 0.5 * (ORDER - 1) / sample_rate[0]
    plt.plot(t, buffers[0], label=names[0])
    plt.plot(t - delay, buffers[1], label=names[1])
    plt.plot(t[ORDER - 1:] - delay, buffers[1][ORDER - 1:])

    plt.legend()
    plt.xlabel("t, s")
    plt.ylabel("y(t)")
    plt.show()

    
if __name__ == "__main__":
    args = parse_arguments()
    buffers: list[list[float]] = read_file(args.files)
    plot(buffers, args.files, args.fs)
