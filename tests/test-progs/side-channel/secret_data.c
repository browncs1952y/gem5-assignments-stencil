#include <stdio.h>
#include <assert.h>
#include <string.h>


int
main(int argc, char **argv)
{
    assert(argc == 2);

    FILE *fptr = fopen(argv[1], "r");
    char secret_data[24];

    while (fgets(secret_data, 24, fptr)) { };

    fclose(fptr);

    if (strcmp(secret_data, "password\n") == 0) {
        for (int i = 0; i < 65536; i++) {
            int x = i * i;
        }
    } else {
        for (int i = 0; i < 65536; i++) {
            int x = i / i;
        }
    }
}
