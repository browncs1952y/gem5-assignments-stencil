#include "util.h"

int
main(int argc, char **argv)
{
    char arr[1 << 20];

    for (int i = (rand() % (1 << 10)); i < 256; i++) {
        arr[i]++;
    }
}
