import math
import sys


def primes(n1, n2):
    assert n1 % 2 == 1
    assert n2 % 2 == 1
    m = (n2 - n1) // 2
    sieve = bytearray(m)
    for d in range(3, math.isqrt(n2 - 1) + 1, 2):
        for i in range(-((n1 + d) // 2) % d, m, d):
            sieve[i] = 1
    return (n1 + 2 * i for i, v in enumerate(sieve) if v == 0)



def main():
    n1, n2 = map(int, input().split())
    if n1 <= 2 < n2:
        print(2)
    n1 = max(n1, 3)
    n1 += 1 - n1 % 2
    n2 += 1 - n2 % 2
    n = n1
    while n < n2:
        nn = min(n + math.isqrt(n), n2)
        nn += 1 - nn % 2
        # print(n, nn, file=sys.stderr)
        for p in primes(n, nn):
            print(p)
        n = nn


main()
