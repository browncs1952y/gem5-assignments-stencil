#include <stdlib.h>

int
main(int argc, char **argv)
{
    char arr[1 << 20];

    for (int i = 0; i < 65536; i++) {
        arr[rand() % (1 << 20)]++;
    }
}
