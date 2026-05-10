# modelled after answer https://ru.stackoverflow.com/a/1625454/416121 by FoxFox
import gmpy2


def main():
    for n in range(*map(int, input().split())):
         if gmpy2.is_prime(n, 34): # ключевая команда в этом коде, всё остальное - обслуга
             print(n)


main()
