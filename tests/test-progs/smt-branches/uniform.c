#include <stdio.h>
#include <stdint.h>

#define N 10000

static inline uint32_t hash(uint32_t x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

int main() {
    volatile int sum = 0;

    uint32_t x = 1;

    for (int i = 0; i < N; i++) {
        x = hash(x);

        if (x >= 1) {
            sum += i;
        } else {
            sum -= i;
        }
    }

    printf("sum = %d\n", sum);
    return 0;
}