#include <stdlib.h>

int
main(int argc, char **argv)
{
    char arr[256];

    for (int i = 0; i < 256; i++) {
        arr[i] = (char) (rand() % 256);
    }

    return 0;
}
