import sys

sys.path.append('tools')

import collections
import numpy as np

import benchmark_app


def avg_time(n):
    samples = (
        (            100000000,             110000000, 1.62),
        (           1000000000,            1010000000, 1.61),
        (          10000000000,           10010000000, 1.82),
        (         100000000000,          100010000000, 1.77),
        (        1000000000000,         1000010000000, 1.81),
        (       10000000000000,        10000010000000, 1.80),
        (      100000000000000,       100000010000000, 1.78),
        (     1000000000000000,      1000000010000000, 1.80),
        (    10000000000000000,     10000000010000000, 1.93),
        (   100000000000000000,    100000000010000000, 1.95),
        (  1000000000000000000,   1000000000010000000, 2.02),
        ( 10000000000000000000,  10000000000010000000, 2.47),
        (100000000000000000000, 100000000000010000000, 4.55)
    )

    x = tuple((n1 + n2) / 2 for n1, n2, _ in samples)
    y = tuple(t / (n2 - n1) for n1, n2, t in samples)

    return np.interp(n, x, y)


def bsearch(low, high, pred):
    if low >= high or pred(low):
        return low
    while low < high - 1:
        mid = (low + high) // 2
        if pred(mid):
            high = mid
        else:
            low = mid
    return high


def find_dt(n):
    v1 = avg_time(n)
    return bsearch(
        1,
        3,
        lambda dt: (avg_time(n + round(dt / v1)) - v1) / v1 > 0.01
    )


def make_scheme():
    scheme = collections.Counter()
    for n1 in map(round, np.geomspace(1e6, 1e20, 1000)):
        dt = find_dt(n1)
        if dt >= 2:
            scheme.update({(n1, n1 + round(dt / avg_time(n1))): 1})
    return scheme


benchmark_app.main(
    make_scheme(),
    lambda n1, n2: f'echo {n1} {n2} | python foxfox/primes.py > temp/foxfox.txt'
)
