#include <stdlib.h>

int
main(int argc, char **argv)
{
    char arr[1 << 20];

    for (int i = (rand() % (1 << 10)); i < 65536; i++) {
        arr[(i * 64) % (1 << 20)]++;
    }
}
