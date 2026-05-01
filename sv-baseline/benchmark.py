import sys

sys.path.append('tools')

import collections
import numpy as np

import curve
import benchmark_app


def avg_time(n):
    return curve.curve(n, 2.016e-8)


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
        60,
        lambda dt: (avg_time(n + round(dt / v1)) - v1) / v1 > 0.01
    )


def make_scheme():
    scheme = collections.Counter()
    for n1 in map(round, np.geomspace(10, 1e17, 250)):
        dt = find_dt(n1)
        if dt >= 2:
            scheme.update({(n1, n1 + round(dt / avg_time(n1))): 1})
    return scheme


benchmark_app.main(make_scheme())
