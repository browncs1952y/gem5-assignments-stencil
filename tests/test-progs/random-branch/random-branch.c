#include <stdlib.h>

int
main(int argc, char **argv)
{
    for (int i = 0; i < 8192; i++) {
label:
        if (rand() % 2 == 0) {
            goto exititeration;
        } else {
            goto label;
        }
exititeration:
        asm("nop");
    }
}
