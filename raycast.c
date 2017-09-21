#include "ppmrw.h"
#include "raycast.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

static void die(const char *reason, ...)
{
    va_list args;
    va_start(args, reason);
    vfprintf(stderr, reason, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if (argc != 5) {
        die("Usage:\t%s [width] [height] [input] [output]", argv[0]);
    }

    FILE *input, *output;
    s32 width = atoi(argv[1]);
    s32 height = atoi(argv[2]);
    char *infn = argv[3];
    char *outfn = argv[4];

    if (width < 0 || height < 0) {
        die("Error: invalid dimensions for output image (%d %d)!", width, height);
    }

    input = fopen(infn, "r");
    if (!input) {
        die("Error: failed to open input file (%s)!", infn);
    }

    struct file_contents fc = get_file_contents(input);
    fclose(input);


    free(fc.memory);
    return 0;
}
