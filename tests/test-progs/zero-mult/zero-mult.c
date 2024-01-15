#include <stdio.h>

int
main(int argc, char **argv)
{
    int x = 5;
    int y = 1;

    for (int i = 0; i < 10000; i++) {
        printf("%d * %d = %d\n", x, y, x * y);
	x++;
    }

    return 0;
}
