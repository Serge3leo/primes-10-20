import math


def main():
    n1, n2 = map(int, input().split())
    n1 = max(2, n1)
    if n1 <= 2 < n2:
        print(2)
    n1 += 1 - n1 % 2
    assert n1 % 2 == 1
    for n in range(n1, n2, 2):
        if all(n % d for d in range(3, math.isqrt(n) + 1, 2)):
            print(n)


main()
