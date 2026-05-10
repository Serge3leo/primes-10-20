import sys

sys.path.append('tools')

import collections
import numpy as np

import benchmark_app


# test runs show mostly constant speed about 200000
# 10000000 11000000 9.94
# 100000000 101000000 8.94
# 1000000000 1001000000 7.90
# 10000000000 10001000000 8.32
# 100000000000 100001000000 8.03
# 1000000000000 1000001000000 7.23
# 10000000000000 10000001000000 7.62
# 100000000000000 100000001000000 7.51
# 1000000000000000 1000000001000000 7.07
# 10000000000000000 10000000001000000 7.21
# 100000000000000000 100000000001000000 7.34
# 1000000000000000000 1000000000001000000 6.93
# 10000000000000000000 10000000000001000000 7.09
# 100000000000000000000 100000000000001000000 3.99
def avg_time(n):
    return 5 / 1000000


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
    lambda n1, n2: f'temp/pakuula {n1} {n2} > temp/pakuula.txt'
)
