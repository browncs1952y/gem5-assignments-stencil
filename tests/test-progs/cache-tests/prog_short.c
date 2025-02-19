#include "util.h"

int
main(int argc, char **argv)
{
    char arr[256];

    for (int i = 0; i < 256; i++) {
        arr[i] = (rand() % (1 << 16));
    }
}
