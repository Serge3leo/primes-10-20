import os
import time


def elapsed(n1, n2):
    start = time.perf_counter()
    os.system(f'echo {n1} {n2} | python sv-baseline/primes.py > temp/sv-baseline.txt')
    finish = time.perf_counter()
    return finish - start


def main():
    n, m, k = map(int, input().split())

    for _ in range(k):
        for i in range(n):
            n1 = m * i
            n2 = n1 + m
            print(n1, elapsed(n1, n2), flush=True)


main()
