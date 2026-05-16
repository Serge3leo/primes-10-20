import numpy as np
import os
import time


def elapsed(s, n2):
    start = time.perf_counter()
    ec = os.system(
        f'python danis/primes.py temp/primes_less_10_10.bin {s} {n2 - s} {n2} > temp/danis.txt'
    ) 
    finish = time.perf_counter()
    if ec != 0:
        exit(1)
    return finish - start


def main():
    for k in range(14, 21):
        n2 = 10 ** k
        s1 = n2 ** 0.5 / 10
        s2 = n2 ** 0.5 * 1000
        for s in map(round, np.geomspace(s1, s2, 20)):
            print(n2, s, elapsed(s, n2) / s * 1e9, flush=True)


main()
