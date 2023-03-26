#!/usr/bin/python3

import argparse as arg
import pylab


def read_file(name: str) -> list[float]:
    buffer: list[str] = []
    with open(name) as file:
        for line in file.readlines():
            data = float(line)
            buffer.append(data)
    return buffer


def plot(buffer: list[float]) -> None:
    pylab.figure(1)
    pylab.plot(buffer, linewidth=2)
    pylab.xlabel("i")
    pylab.ylabel("y(i)")


if __name__ == "__main__":
    parser: arg.ArgumentParser = arg.ArgumentParser()
    parser.add_argument("--file", type=str)
    args: arg.Namespace = parser.parse_args()

    buffer: list[float] = read_file(args.file)
    plot(buffer)
