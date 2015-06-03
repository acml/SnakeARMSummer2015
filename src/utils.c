#include "utils.h"

/*  
 * Allocate memory for a new state_t
 * Initialises all the values in the struct
 * Returns a pointer to the state_t
 */
state_t *newState(void) {
    state_t *state = malloc(sizeof(state_t));
    if (state == NULL) {
        return NULL;
    }

    state->decoded = malloc(sizeof(decoded_t));
    if (state->decoded == NULL) {
        free(state);
        return NULL;
    }

    state->registers = malloc(REGISTERS_COUNT * sizeof(uint32_t));
    if (state->registers == NULL) {
        free(state->decoded);
        free(state);
        return NULL;
    }
    memset(state->registers, 0, REGISTERS_COUNT * sizeof(uint32_t));

    state->memory = malloc(MEMORY_SIZE * sizeof(uint8_t));
    if (state->memory == NULL) {
        free(state->decoded);
        free(state->registers);
        free(state);
        return NULL;
    }
    memset(state->memory, 0, MEMORY_SIZE * sizeof(uint8_t));

    state->fetched = 0;
    state->isDecoded = 0;
    state->isFetched = 0;
    state->isTermainated = 0;

    return state;
}

/*  
 * Checks for null pointers
 * Frees all the pointers inside the state
 * Finally frees state itself
 */ 
void delState(state_t *state) {
    if (state != NULL) {
        if (state->decoded != NULL) {
            free(state->decoded);
        }
        
        if (state->registers != NULL) {
            free(state->registers);
        }
        
        if (state->memory != NULL) {
            free(state->memory);
        }
        
        free(state);
    }
}

/*
 * Takes in the program's arguments
 * Opens and reads the file specified
 * Writes this data to the state
 * Closes file if opened, and returns exit codes
 * Return 0 on failure, return EXIT_SUCCESS on success
 */ 
int readBinary(state_t *state, int argc, char **argv) {
    if (argc == 1) {
        printf("No input file specified.\n");
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("Could not open input file.\n");
        return EXIT_FAILURE;
    }

    uint8_t *buffer = malloc(sizeof(uint8_t));
    int i = 0;
    while (fread(buffer, sizeof(uint8_t), 1, fp) == 1) {
        state->memory[i] = *buffer;
        i++;
    }
    free(buffer);

    fclose(fp);
    return EXIT_SUCCESS;
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

    return EXIT_SUCCESS;
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

/*
 * Returns the value in data between and upper and lower bit(both included).
 * Because it is a short function called often, we have decided to inline it
 * to improve performance.
 */
inline uint32_t maskBits(uint32_t data, int upper, int lower) {
    assert(upper >= lower && upper <= 31 && lower >= 0);
    data <<= TOP_BIT - upper;
    data >>= TOP_BIT - (upper - lower);
    return data;
}

/*
 * Returns the given flag from CPSR_REG
 */
uint32_t getFLag(state_t *state, int flag) {
    return maskBits(state->registers[CPSR_REG], flag, flag);
}

/*
 * Sets or clears(according to val) given flag in CPSR_REG. 
 */
void setFlag(state_t *state, int val, int flag) {
    assert(val == 0 || val == 1);
    state->registers[CPSR_REG] ^= (-val ^ state->registers[CPSR_REG])
            & (1 << flag);
}
