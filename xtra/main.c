#include <stdio.h>
#include <sys/stat.h>

#include "xtra.h"

#define IMAGE_ARG 1

int main(int argc, char **argv) {
    FILE *fp;
    struct stat fs;

    if (argc < 2) {
        printf("usage: xtra <obj file>\n");
        printf("      <image file> xis object file created by xas or xld\n");
        return 1;
    }

    if (stat(argv[IMAGE_ARG], &fs)) {
        printf("error: could not stat image file %s\n", argv[IMAGE_ARG]);
        return 1;
    }

    fp = fopen(argv[IMAGE_ARG], "rb");
    if (!fp) {
        printf("error: could not open image file %s\n", argv[IMAGE_ARG]);
        return 1;
    }

    xtra("test", fp, fs.st_size);
    fclose(fp);
    return 0;
}


