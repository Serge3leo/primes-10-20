// gcc -std=c11 -pedantic -Wall -Wextra -Werror -Wwrite-strings -Wconversion primes.c

#include <string.h>

#include <primes.h>

uint8_t *small_primes(uint64_t n) {
    n += 1 - (n & 1);
    assert((n & 1) == 1);
    uint64_t s = (n - 3) >> 1;
    bool *sieve = malloc(s);
    if (sieve == NULL) {
        exit(1);
    }
    memset(sieve, true, s);
    const uint32_t p_last = i64_isqrt(n - 1) + 1;

    for (uint32_t p = 3; p < p_last; p += 2) {
        uint64_t i0 = (p - 3) >> 1;
        if (sieve[i0]) {
            for (uint64_t i = i0 + p; i < s; i += p) {
                sieve[i] = false;
            }
        }
    }

    unsigned m = 0;
    uint64_t prev_i = 0;
    uint64_t max_diff = 0;
    for (uint64_t i = 0; i < s; ++i) {
        if (sieve[i]) {
            ++m;
            if (i - prev_i > max_diff) {
                max_diff = i - prev_i;
            }
            prev_i = i;
            // printf("%" PRIu64 "\n", 3 + 2 * i);
        }
    }
    printf("%u %" PRIu64 "\n", m, max_diff);
    
    free(sieve);
    return NULL;
}

void fill_sieve(const Int86 *n, uint64_t s, bool sieve[/* s */]) {
    memset(sieve, true, s);
    Int86 n1 = *n; add(&n1, 1); shr(&n1);
    Int86 n2 = *n; add(&n2, 2 * s);
    const uint64_t p_last = i86_isqrt(&n2) + 1;
    for (uint64_t p = 3; p < p_last; p += 2) {
        add(&n1, 1); // n1 = n + p
        uint64_t i0 = p - mod(&n1, p);
        if (i0 == p) {
            i0 = 0;
        }
        for (uint64_t i = i0; i < s; i += p) {
            sieve[i] = false;
        }
    }
}

// TODO: it is slow
void print_sieve(const Int86 *n, uint64_t s, bool sieve[/* s */]) {
    for (uint64_t i = 0; i < s; ++i) {
        if (sieve[i]) {
            Int86 p = *n;
            add(&p, 2 * i);
            print(stdout, &p);
            puts("");
        }
    }
}

void count_sieve(uint64_t s, bool sieve[/* s */]) {
    uint64_t c = 0;
    for (uint64_t i = 0; i < s; ++i) {
        if (sieve[i]) {
            ++c;
        }
    }
    printf("%" PRIu64 "\n", c);
}

void sift(const Int86 *n, uint64_t s) {
    assert((n->low & 1) != 0);
    assert((s & 1) == 0);
    s >>= 1;
    bool *sieve = malloc(s);
    if (sieve == NULL) {
        exit(1);
    }
    fill_sieve(n, s, sieve);
    print_sieve(n, s, sieve);
    // count_sieve(s, sieve);
    free(sieve);
}

int main(int argc, char *argv[]) {
    Int86 n1; scan(&n1);
    Int86 n2; scan(&n2);

    small_primes(i86_isqrt(&n2) + 1);
    return 0;

    bool run_sift = (argc == 1) || (argv == NULL);

    // print(stdout, &n1);
    // printf(" ");
    // print(stdout, &n2);
    // puts("");
    for (; ; ) {
        n1.low += (n1.low & 1) == 0; // make it next odd
        uint64_t s = i86_isqrt(&n1);
        s += s & 1;
        Int86 n = n1;
        add(&n, s);
        if (lt(&n, &n2)) {
            print(stderr, &n1);
            fprintf(stderr, " ");
            print(stderr, &n);
            fprintf(stderr, "\n");
            if (run_sift) {
                sift(&n1, s);
            }
            n1 = n;
        } else {
            s = sub(&n2, &n1);
            s -= s & 1;
            print(stderr, &n1);
            fprintf(stderr, " ");
            print(stderr, &n2);
            fprintf(stderr, "\n");
            if (run_sift) {
                sift(&n1, s);
            }
            break;
        }
    }
}
