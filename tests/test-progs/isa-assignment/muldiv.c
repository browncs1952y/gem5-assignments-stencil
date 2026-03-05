#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char **argv)
{
    uint64_t seed = 1;
    if (argc > 1) {
        seed = strtoull(argv[1], NULL, 10);
    }

    srand((unsigned)seed);

    int64_t x = rand() % 65536 - 32768;
    int64_t y = rand() % 1024 + 1;

    for (int i = 0; i < 20; i++) {
        printf("%d * %d = %d\n", x, y, x * y);
        printf("%d / %d = %d\n", x, y, x / y);
	    y++;
    }

    return 0;
}
