import sys

sys.path.append('tools')

import numpy as np

import benchmark_app


s = 10 ** 10
scheme = {
    (n2 - s, n2): 1
    for n2 in map(round, np.geomspace(1e11, 1e20, 200))
}

benchmark_app.main(
    scheme,
    lambda n1, n2: f'python danis/primes.py temp/primes_less_10_10.bin {s} {n1} {n2} > temp/danis.txt'
)
