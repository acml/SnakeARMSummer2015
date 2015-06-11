#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "binarywriter.h"
#include "definitions.h"

/*
 * Function writes memory positions and related data from the given file
 */
void writeBinary(char **argv, uint8_t *memory, uint32_t memoryLength) {
    FILE *fp = fopen(argv[2], "wb");
    if (fp == NULL) {
        printf("Could not open output file.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < memoryLength; i++) {
        fwrite(&memory[i], sizeof(uint8_t), 1, fp);
    }

    fclose(fp);
}

/*
 * Function stores given word(encoded instruction) to the memory
 */
void storeWord(uint8_t *memory, uint32_t address, uint32_t word) {
    for (int i = 0; i < BYTES_IN_WORD; i++) {
        memory[address + i] = (uint8_t) word;
        word >>= BITS_IN_BYTE;
    }
}
