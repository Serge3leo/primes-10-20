#include <stdio.h>
#include <gmp.h>

int main() {
    mpz_t n;
    mpz_init(n);
    mpz_inp_str(n, stdin, 10);

    mpz_t n2;
    mpz_init(n2);
    mpz_inp_str(n2, stdin, 10);

    for (; mpz_cmp(n, n2) < 0; mpz_add_ui(n, n, 1)) {
        if (mpz_probab_prime_p(n, 34)) {
            mpz_out_str(stdout, 10, n);
            putchar('\n');
        }
    }

    mpz_clear(n2);
    mpz_clear(n);
}
