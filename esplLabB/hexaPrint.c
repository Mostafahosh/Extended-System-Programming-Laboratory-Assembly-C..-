#include <stdio.h>
#include <stdlib.h>

#define CHUNK_SIZE 16  

void PrintHex(unsigned char *buffer, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02X", buffer[i]);
        if (i < length - 1)
            printf(" ");
    }
}

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    unsigned char buffer[CHUNK_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, CHUNK_SIZE, file)) > 0) {
        PrintHex(buffer, bytesRead);
        if (!feof(file))
            printf(" ");
    }

    printf("\n");
    fclose(file);
    return 0;
}