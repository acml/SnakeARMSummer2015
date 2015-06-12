#include <stdlib.h>
#include <stdio.h>

#include "inputoutput.h"

void printRegister(state_t *state, int reg);

/*
 * Takes in the program's arguments
 * Opens and reads the file specified
 * Writes this data to the state
 * Closes file if opened, and returns exit codes
 * Return 0 on failure, return 1 on success
 */
int readBinary(state_t *state, int argc, char **argv) {
    if (argc == 1) {
        printf("No input file specified.\n");
        return 0;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("Could not open input file.\n");
        return 0;
    }

    uint8_t *buffer = malloc(sizeof(uint8_t));
    if (buffer == NULL) {
        return 0;
    }
    int i = 0;
    while (fread(buffer, sizeof(uint8_t), 1, fp) == 1) {
        state->memory[i] = *buffer;
        i++;
    }
    free(buffer);

    fclose(fp);
    return 1;
}

/*
 * Prints the content of the state's registers
 * Prints the content of memory at the given state
 * Prints the above to standard out and returns 1 to indicate success
 */
int outputState(state_t *state) {
    printf("Registers:\n");
    for (int i = 0; i < REGISTERS_COUNT; i++) {
        if (i != SP_REG && i != LR_REG) {
            printRegister(state, i);
        }
    }

    printf("Non-zero memory:\n");
    for (int i = 0; i < MEMORY_SIZE; i += BYTES_IN_WORD) {
        if (((uint32_t *) state->memory)[i / BYTES_IN_WORD] != 0) {
            printf("0x%08x: 0x", i);
            for (int j = 0; j < BYTES_IN_WORD; j++) {
                printf("%02x", state->memory[i + j]);
            }
            printf("\n");
        }
    }

    return 1;
}

/*
 * Prints the contents of the register in a state
 * If it is r15 or r16, it prints label PC or CPSR respectively
 */
void printRegister(state_t *state, int reg) {
    if (reg == PC_REG) {
        printf("PC  ");
    } else if (reg == CPSR_REG) {
        printf("CPSR");
    } else {
        printf("$%-3d", reg);
    }
    printf(": %10d (0x%08x)\n", state->registers[reg], state->registers[reg]);
}
