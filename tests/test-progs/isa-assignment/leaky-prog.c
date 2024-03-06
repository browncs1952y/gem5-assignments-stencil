#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, const char **argv)
{
    if (argc < 2) {
        printf("No input data! Exiting\n");
        return -1;
    }

    unsigned long long r1 = 1804289383ULL;
    unsigned long long r2 = 846930886ULL;

    unsigned long long count = 0;
    if (strcmp(argv[1], "supersecretdata") == 0) {
        for (int i = 0; i < 65536; i++) {
            count += r1 * r2;
        }
    } else if (strcmp(argv[1], "notsosecretdata") == 0) {
        for (int i = 0; i < 65536; i++) {
            count += r1 / r2;
        }
    } else {
        for (int i = 0; i < 65536; i++) {
            count += r1 * 0;
        }
    }

    return 0;
}
