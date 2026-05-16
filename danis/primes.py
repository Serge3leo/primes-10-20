import bisect
import math
import os
import sys
from typing import Generator, Iterable, Sequence

num_threads = "1"
os.environ["OMP_NUM_THREADS"] = num_threads
os.environ["OPENBLAS_NUM_THREADS"] = num_threads
os.environ["MKL_NUM_THREADS"] = num_threads
os.environ["VECLIB_MAXIMUM_THREADS"] = num_threads
os.environ["NUMEXPR_NUM_THREADS"] = num_threads

import numpy as np


def _to_seq(x: Iterable[int]) -> Sequence[int]:
    if isinstance(x, (list, np.ndarray)):
        return x
    return list(x)


def sieve(primes_less_10_10, a: int, b: int, sieve_size: int) -> Generator[Iterable[int], None, None]:
    # генерирует последовательности целых чисел (int | np.int64)
    # их обьединение есть все простые из [a, b) в порядке возрастания

    assert 0 <= a < b
    assert sieve_size > 0
    if b <= 10:
        primes = [2, 3, 5, 7]
        yield [p for p in primes if p < b]
        return
    if primes_less_10_10 is not None and b <= 10**10+10:  # 10^10 + 10 < nextprime(10^10)
        id1 = bisect.bisect_left(primes_less_10_10, a)
        id2 = bisect.bisect_left(primes_less_10_10, b)
        yield primes_less_10_10[id1:id2]
        return

    groups = [_to_seq(group) for group in sieve(primes_less_10_10, 0, math.isqrt(b)+1, sieve_size)]
    if a <= groups[-1][-1]:
        yield (
            p
            for group in groups
            for p in group
            if a <= p
        )

    mask = np.empty(sieve_size, dtype="uint8")
    for i in range(max(math.isqrt(b), a), b, sieve_size):
        size = min(sieve_size, b - i)
        # print(i, i + size, file=sys.stderr)
        mask = mask[:size]
        mask[:] = 1
        for group in groups:
            for p in group:
                p = int(p)
                mask[(-i) % p::p] = 0
        if size + i >= 2**63:
            yield (int(j) + i for j in mask.nonzero()[0])
        else:
            yield mask.nonzero()[0] + i


# sieve_size = 8*1024**2
# 8MB. Оптимально на моём ноутбуке (L2: 6*512KB, L3: 16MB)
# Возможно равен L2, честно не уверен

# primes_less_10_10 = None
# Разрешить предвычислить все простые до 10^10
# Занимает менее 4ГБ


def main():
    primes_file, sieve_size, n1, n2 = sys.argv[1:]
    sieve_size = int(sieve_size)
    n1 = int(n1)
    n2 = int(n2)

    if not os.path.exists(primes_file):
        with open(primes_file, "wb") as out:
            for group in sieve(None, 0, 10**10, sieve_size):
                if isinstance(group, np.ndarray):
                    out.write(group.astype(np.int64).tobytes())
                else:
                    for p in group:
                        out.write(int(p).to_bytes(length=8, byteorder="little", signed=True))
    primes_less_10_10 = np.memmap(primes_file, dtype=np.int64).copy()
    if primes_less_10_10.shape[0] != 455052511:
        os.remove(primes_file)
        raise ValueError(f"invalid {primes_file}")

    for group in sieve(primes_less_10_10, n1, n2, sieve_size):
        for n in group:
            print(n)


if __name__ == "__main__":
    main()
