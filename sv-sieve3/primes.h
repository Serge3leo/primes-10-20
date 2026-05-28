#ifndef PRIMES_H_
#define PRIMES_H_

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define LOW_BITS 22
#define LOW_MASK ((1UL << LOW_BITS) - 1UL)

// store 86-bit unsigned integer
typedef struct {
    uint32_t low;   //  0 - 21 bits
    uint64_t high;  // 21 - 85 bits
} Int86;

bool lt(const Int86 *a, const Int86 *b) {
    if (a->high < b->high) { return true ; }
    if (a->high > b->high) { return false; }
    return a->low < b->low;
}

void add(Int86 *a, uint64_t c) {
    const uint64_t low = a->low + c;
    a->low = low & LOW_MASK;
    a->high = a->high + (low >> LOW_BITS);
}

void mul_add(Int86 *a, unsigned b, uint64_t c) {
    const uint64_t low = a->low * b + c;
    a->low = low & LOW_MASK;
    a->high = a->high * b + (low >> LOW_BITS);
}

void shr(Int86 *a) {
    a->low = ((a->high & 1) << (LOW_BITS - 1)) | (a->low >> 1);
    a->high >>= 1;
}

uint64_t sub(const Int86 *a, const Int86 *b) {
    return ((a->high - b->high) << LOW_BITS) + a->low - b->low;
}

uint64_t mod(const Int86 *a, uint64_t b) {
    assert(b < (1UL << 43));
    return (((a->high % b) << LOW_BITS) | a->low) % b;
}

void sqr(Int86 *b, uint64_t a) {
    const uint64_t low = a & LOW_MASK;
    const uint64_t high = a >> LOW_BITS;
    const uint64_t low2 = low * low;
    const uint64_t high2 = high * high;
    const uint64_t low_high = low * high;
    b->low = low2 & LOW_MASK;
    b->high = (low2 >> LOW_BITS) + 2 * low_high + (high2 << LOW_BITS);
}

// TODO: it is slow
uint64_t i86_isqrt(const Int86 *a) {
    uint64_t low = 0;
    uint64_t high = 1UL << 43;

    while (low + 1 < high) {
        uint64_t mid = (low + high) >> 1;
        Int86 b;
        sqr(&b, mid);
        if (lt(a, &b)) {
            high = mid;
        } else {
            low = mid;
        }
    }
    return low;
}

// TODO: it is slow
uint32_t i64_isqrt(uint64_t a) {
    uint32_t low = 0;
    uint32_t high = (uint32_t)-1;

    while (low + 1 < high) {
        uint32_t mid = (low + high) >> 1;
        if (a < ((uint64_t) mid) * mid) {
            high = mid;
        } else {
            low = mid;
        }
    }
    return low;
}


#define DEC_BASE 1000000000000UL

// store integers in range [0, 10**24)
typedef struct {
    uint64_t low;   // low 12 decimal digits
    uint64_t high;  // high 12 decimal digits
} DecInt24;


void to_decimal(const Int86 *a, DecInt24 *b) {
    uint64_t low = (a->high % DEC_BASE) * (1UL << LOW_BITS) + a->low;
    uint64_t high = (a->high / DEC_BASE) * (1UL << LOW_BITS);
    b->low = low % DEC_BASE;
    b->high = high + low / DEC_BASE;
}

void print(FILE *f, const Int86 *a) {
    DecInt24 b;
    to_decimal(a, &b);
    if (b.high == 0) {
        fprintf(f, "%" PRIu64, b.low);
    } else {
        // depends on DEC_BASE value
        fprintf(f, "%" PRIu64 "%.012" PRIu64, b.high, b.low);
    }
}

int readchar() {
    const int c = getchar();
    if (c == EOF) {
        exit(1);
    }
    return c;
}

void scan(Int86 *a) {
    // skip whitespaces
    for (; ; ) {
        const int c = readchar();
        if (!isspace(c)) {
            ungetc(c, stdin);
            break;
        }
    }

    a->low = 0;
    a->high = 0;

    for (; ; ) {
        const int c = readchar();
        if ('0' <= c && c <= '9') {
            mul_add(a, 10, (unsigned)(c - '0'));
        } else {
            ungetc(c, stdin);
            break;
        }
    }
}

#endif // #ifndef PRIMES_H_
