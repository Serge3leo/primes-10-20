import sys

sys.path.append('tools')

import collections
import numpy as np

import benchmark_app


def make_scheme():
    scheme = collections.Counter()
    s = round(3 / (6.3 / 10000000))
    for n1 in map(round, np.geomspace(1e6, 1e20, 1000)):
        scheme.update({(n1, n1 + s): 1})
    return scheme


benchmark_app.main(
    make_scheme(),
    lambda n1, n2: f'temp/pakuula-2 -i 34 -p {n1} {n2} > temp/pakuula-2.txt'
)
