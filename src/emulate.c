#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define BYTES_IN_WORD 4
#define REGISTERS_SIZE 17
#define MEMORY_SIZE 65536
#define TOP_BIT 31
#define N_BIT 31
#define Z_BIT 30
#define C_BIT 29
#define V_BIT 28
#define I_BIT 25
#define PC_REG 15
#define CPSR_REG 16

typedef struct state {
    uint32_t fetched;
    uint32_t *registers;
    uint32_t *memory;
    int isDecoded;
    int isFetched;
} state_t;

typedef enum {
    DATA_PROCESSING, MULTIPLY, SINGLE_DATA_TRANSFER, BRANCH
} ins_t;

state_t *newState(void);
void delState(state_t *state);
int readBinary(state_t *state, int argc, char **argv);

void execut(state_t *state);
void decode(state_t *state);
void fetch(state_t *state);
void incPC(state_t *state);
uint32_t maskBits(uint32_t data, int upper, int lower);
uint32_t getN(state_t *state);
uint32_t getZ(state_t *state);
uint32_t getC(state_t *state);
uint32_t getV(state_t *state);
void setFlag(state_t *state, int val, int flag);

ins_t insTpye(state_t *state);

int checkCond(state_t *state);

int main(int argc, char **argv) {
    state_t *state = newState();
    if (state == NULL) {
        return EXIT_FAILURE;
    }
    if (!readBinary(state, argc, argv)) {
        return EXIT_FAILURE;
    }

    while (1) {
        execut(state);
        decode(state);
        fetch(state);
        incPC(state);
    }

    return EXIT_SUCCESS;
}

state_t *newState(void) {
    state_t *state = malloc(sizeof(state_t));
    if (state == NULL) {
        return NULL;
    }

    state->registers = malloc(REGISTERS_SIZE * sizeof(uint32_t));
    if (state->registers == NULL) {
        free(state);
        return NULL;
    }
    memset(state->registers, 0, REGISTERS_SIZE);

    state->memory = malloc(MEMORY_SIZE * sizeof(uint32_t));
    if (state->memory == NULL) {
        free(state->registers);
        free(state);
        return NULL;
    }
    memset(state->memory, 0, MEMORY_SIZE);

    state->fetched = 0;
    state->isDecoded = 0;
    state->isFetched = 0;
    return state;
}

void delState(state_t *state) {
    if (state != NULL) {
        free(state->registers);
        free(state->memory);
        free(state);
    }
}

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
    fread(state->memory, sizeof(uint32_t), MEMORY_SIZE, fp);
    fclose(fp);

    return 1;
}

void execut(state_t *state) {
    if (state->isDecoded) {

    }
}

void decode(state_t *state) {
    if (state->isFetched) {

        state->isDecoded = 1;
    }
}

void fetch(state_t *state) {
    state->fetched = state->memory[state->registers[PC_REG]];
    state->isFetched = 1;
}

void incPC(state_t *state) {
    state->registers[PC_REG] += BYTES_IN_WORD;
}

uint32_t maskBits(uint32_t data, int upper, int lower) {
    assert(upper >= lower && upper <= 31 && lower >= 0);
    data <<= TOP_BIT - upper;
    data >>= TOP_BIT - (upper - lower);
    return data;
}

uint32_t getN(state_t *state) {
    return maskBits(state->registers[CPSR_REG], N_BIT, N_BIT);
}

uint32_t getZ(state_t *state) {
    return maskBits(state->registers[CPSR_REG], Z_BIT, Z_BIT);
}

uint32_t getC(state_t *state) {
    return maskBits(state->registers[CPSR_REG], C_BIT, C_BIT);
}

uint32_t getV(state_t *state) {
    return maskBits(state->registers[CPSR_REG], V_BIT, V_BIT);
}

void setFlag(state_t *state, int val, int flag) {
    if (val) {
        state->registers[CPSR_REG] |= 1 << flag;
    } else {
        state->registers[CPSR_REG] &= ~(1 << flag);
    }
}

ins_t insTpye(state_t *state) {
    uint32_t typeBits = maskBits(state->fetched, 27, 26);
    if (typeBits == 1) {
        return SINGLE_DATA_TRANSFER;
    } else if (typeBits == 2) {
        return BRANCH;
    } else {
        uint32_t bitI = maskBits(state->fetched, I_BIT, I_BIT);
        uint32_t bit4 = maskBits(state->fetched, 4, 4);
        uint32_t bit7 = maskBits(state->fetched, 7, 7);
        if (bitI == 0 && bit4 == 1 && bit7 == 1) {
            return MULTIPLY;
        } else {
            return DATA_PROCESSING;
        }
    }
}

int checkCond(state_t *state) {
    uint32_t condBits = maskBits(state->fetched, N_BIT, V_BIT);
    switch (condBits) {
        case 0:
            return getZ(state);
        case 1:
            return !getZ(state);
        case 10:
            return getN(state) == getV(state);
        case 11:
            return getN(state) != getV(state);
        case 12:
            return !getZ(state) && getN(state) == getV(state);
        case 13:
            return getZ(state) || getN(state) != getV(state);
        default:
            return 1;
    }
}
