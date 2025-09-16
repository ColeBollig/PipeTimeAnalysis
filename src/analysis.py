#!/usr/bin/env python3

import sys
from statistics import geometric_mean as gmean

def make_int(var):
    return int(var.strip())

def median(data):
    return data[((len(data) - 1) / 2) + 1] if len(data) % 2 == 1 else (data[len(data)] + data[len(data) + 1]) / 2

with open(sys.argv[1], "r") as f:
    total_bytes = list()
    total_seconds = list()
    seconds_per_bytes = dict()

    for line in f:
        num_bytes, nanoseconds = line.split(":")[1].split("|")

        num_bytes = make_int(num_bytes)
        nanoseconds = make_int(nanoseconds)

        if num_bytes > 524288:
            continue

        total_bytes.append(num_bytes)
        total_seconds.append(nanoseconds)

        if num_bytes in seconds_per_bytes:
            seconds_per_bytes[num_bytes].append(nanoseconds)
        else:
            seconds_per_bytes[num_bytes] = [nanoseconds]

    b = gmean(total_bytes)
    ns = gmean(total_seconds)
    throughput = b / (ns / 1000)

    print(f"Average Throughput: {throughput:.2f} B/ms")
    for num_bytes, nanoseconds in seconds_per_bytes.items():
        avg = (gmean(nanoseconds)) / 1000
        print(f"{num_bytes:7d} bytes: {avg:.2f} microseconds")
        put = gmean([num_bytes for i in range(len(nanoseconds))]) / avg
        print(f"               {put:.2f} B/ms")
