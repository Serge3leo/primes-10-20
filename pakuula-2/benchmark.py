import sys

sys.path.append('tools')

import collections
import numpy as np

import benchmark_app


def avg_time(n):
    return 6.3 / 10000000


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
    lambda n1, n2: f'temp/pakuula-2 {n1} {n2} > temp/pakuula-2.txt'
)
