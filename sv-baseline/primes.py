def is_prime(n):
    return n > 1 and all(n % d for d in range(2, n))


def main():
    n1, n2 = map(int, input().split())
    for n in range(n1, n2):
        if is_prime(n):
            print(n)


main()
