#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "definitions.h"
#include "twopasses.h"
#include "binarywriter.h"

/*
 * Allocates memory, sets it to 0
 * assembles the program and writes the binary file
 * then frees the memory
 */
int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Input file and/or output file not specified.\n");
        exit(EXIT_FAILURE);
    }

    uint8_t *memory = malloc(MEMORY_SIZE);
    if (memory == NULL) {
        perror("memory");
        exit(EXIT_FAILURE);
    }
    memset(memory, 0, MEMORY_SIZE);

    uint32_t memoryLength = assembly(argv, memory);
    writeBinary(argv, memory, memoryLength);

    free(memory);
    return EXIT_SUCCESS;
}
