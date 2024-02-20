#include <stdlib.h>

int
main(int argc, char **argv)
{
    char arr[512 << 10];

    for (int i = 0; i < 65536; i++) {
        arr[rand() % (512 << 10)]++;
    }
}
