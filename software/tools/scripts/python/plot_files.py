#!/usr/bin/python3

import argparse as arg
import numpy as np
import matplotlib.pyplot as plt


def parse_arguments() -> arg.Namespace:
    parser: arg.ArgumentParser = arg.ArgumentParser()
    parser.add_argument("--files", type=str, nargs='*')
    parser.add_argument("--fs", type=float)
    return parser.parse_args()


def read_files(file_names: str) -> list[list[float]]:
    buffers: list[float] = []
    for file in file_names:
        with open(file) as f:
            file_data: list[float] = []
            for line in f.readlines():
                sample = float(line)
                file_data.append(sample)
            buffers.append(file_data)
    return buffers


def plot(buffers: list[list[float]], names: list[str], sampling_rate: float) -> None:
    font = {'size': 16}
    plt.rc('font', **font)

    t = np.arange(len(buffers[0])) / sampling_rate

    for buffer, name in zip(buffers, names):
        plt.plot(t, buffer, label=name, linewidth=2)

    plt.legend()
    plt.xlabel("t, s")
    plt.ylabel("e(t)")
    plt.grid()
    plt.show()


if __name__ == "__main__":
    args = parse_arguments()
    buffers: list[list[float]] = read_files(args.files)

    plot(buffers, args.files, args.fs)
