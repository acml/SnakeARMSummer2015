#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <endian.h>

#define BYTES_IN_WORD 4
#define BITS_IN_WORD 32
#define REGISTERS_COUNT 17
#define MEMORY_SIZE 65536
#define TOP_BIT 31
#define N_BIT 31
#define Z_BIT 30
#define C_BIT 29
#define V_BIT 28
#define I_BIT 25
#define S_BIT 20
#define A_BIT 21
#define P_BIT 24
#define U_BIT 23
#define L_BIT 23
#define SP_REG 13
#define LR_REG 14
#define PC_REG 15
#define CPSR_REG 16

typedef enum {
    DATA_PROCESSING, MULTIPLY, SINGLE_DATA_TRANSFER, BRANCH, TERMINATION
} ins_t;

typedef enum {
    EQ, NE, GE, LT, GT, LE, AL
} cond_t;

typedef enum {
    AND, EOR, SUB, RSB, ADD, TST, TEQ, CMP, ORR, MOV
} opcode_t;

typedef enum {
    LSL, LSR, ASR, ROR
} shift_t;

typedef struct arm_decoded {
    ins_t ins;
    u_int32_t rd;
    u_int32_t rn;
    u_int32_t rs;
    u_int32_t rm;
    cond_t cond;
    opcode_t opcode;
    shift_t shift;
    u_int32_t shiftValue;
    u_int32_t immValue;
    u_int32_t branchOffest;
    int isI;
    int isS;
    int isA;
    int isP;
    int isU;
    int isL;
} decoded_t;

typedef struct arm_state {
    decoded_t *decoded;
    uint32_t fetched;
    uint32_t *registers;
    uint32_t *memory;
    int isDecoded;
    int isFetched;
    int isTermainated;
} state_t;

state_t *newState(void);
void delState(state_t *state);
int readBinary(state_t *state, int argc, char **argv);
int writeState(state_t *state, char **argv);
void printRegister(state_t *state, FILE *fp, int reg);

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

ins_t insTpye(uint32_t ins);
cond_t condTpye(uint32_t ins);
opcode_t opcodeType(uint32_t ins);
shift_t shiftType(uint32_t ins);

int checkCond(state_t *state);
u_int32_t shiftData(u_int32_t data, shift_t shift, u_int32_t shiftValue);
int shiftCarry(u_int32_t data, shift_t shift, u_int32_t shiftValue);

int main(int argc, char **argv) {
    state_t *state = newState();
    if (state == NULL) {
        return EXIT_FAILURE;
    }

    if (!readBinary(state, argc, argv)) {
        return EXIT_FAILURE;
    }

    while (!state->isTermainated) {
        execut(state);
        decode(state);
        fetch(state);
        incPC(state);
    }

    if (!writeState(state, argv)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

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

    state->memory = malloc(MEMORY_SIZE * sizeof(uint32_t));
    if (state->memory == NULL) {
        free(state->decoded);
        free(state->registers);
        free(state);
        return NULL;
    }
    memset(state->memory, 0, MEMORY_SIZE * sizeof(uint32_t));

    state->fetched = 0;
    state->isDecoded = 0;
    state->isFetched = 0;
    state->isTermainated = 0;
    return state;
}

void delState(state_t *state) {
    if (state != NULL) {
        free(state->decoded);
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

    uint32_t *word = malloc(sizeof(uint32_t));
    if (word == NULL) {
        return 0;
    }
    int i = 0;
    while (fread(word, sizeof(uint32_t), 1, fp)) {
        state->memory[i] = be32toh(*word);
        i++;
    }
    free(word);

    fclose(fp);
    return 1;
}

int writeState(state_t *state, char **argv) {
    FILE *fp = fopen(strcat(argv[1], ".out"), "w");
    if (fp == NULL) {
        printf("Could not open output file.\n");
        return 0;
    }

    fprintf(fp, "Registers:\n");
    for (int i = 0; i < REGISTERS_COUNT; i++) {
        if (i != SP_REG && i != LR_REG) {
            printRegister(state, fp, i);
        }
    }

    fprintf(fp, "Non-zero memory:\n");
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (state->memory[i] != 0) {
            fprintf(fp, "0x%08x: 0x%08x\n", i * BYTES_IN_WORD,
                    state->memory[i]);
        }
    }

    fclose(fp);
    return 1;
}

void printRegister(state_t *state, FILE *fp, int reg) {
    if (reg == PC_REG) {
        fprintf(fp, "PC\t:");
    } else if (reg == CPSR_REG) {
        fprintf(fp, "CPSR:");
    } else {
        fprintf(fp, "$%d\t:", reg);
    }
    fprintf(fp, "% 11d (0x%08x)\n", state->registers[reg],
            state->registers[reg]);
}

void execut(state_t *state) {
    if (!state->isDecoded) {
        return;
    }
    if (state->decoded->ins == TERMINATION) {
        return;
    }

}

void decode(state_t *state) {
    if (!state->isFetched) {
        return;
    }

    decoded_t *decoded = state->decoded;
    u_int32_t ins = state->fetched;

    decoded->ins = insTpye(ins);

    decoded->rd = maskBits(ins, 15, 12);
    decoded->rn = maskBits(ins, 19, 16);
    if (decoded->ins == MULTIPLY) {
        u_int32_t tmp = decoded->rd;
        decoded->rd = decoded->rn;
        decoded->rn = tmp;
    }
    decoded->rs = maskBits(ins, 11, 8);
    decoded->rm = maskBits(ins, 3, 0);

    decoded->cond = condTpye(ins);
    decoded->opcode = opcodeType(ins);
    decoded->shift = shiftType(ins);
    decoded->shiftValue = maskBits(ins, 11, 7);
    if (decoded->ins == DATA_PROCESSING) {
        //TODO
    } else {
        decoded->immValue = maskBits(ins, 11, 0);
    }
    //TODO decoded->branchOffest =

    decoded->isI = maskBits(ins, I_BIT, I_BIT);
    decoded->isS = maskBits(ins, S_BIT, S_BIT);
    decoded->isA = maskBits(ins, A_BIT, A_BIT);
    decoded->isP = maskBits(ins, P_BIT, P_BIT);
    decoded->isU = maskBits(ins, U_BIT, U_BIT);
    decoded->isL = maskBits(ins, L_BIT, L_BIT);

    state->isDecoded = 1;
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
    if (upper == lower) {
        return (data >> upper) & 1;
    } else {
        data <<= TOP_BIT - upper;
        data >>= TOP_BIT - (upper - lower);
        return data;
    }
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
    state->registers[CPSR_REG] ^= (-val ^ state->registers[CPSR_REG])
            & (1 << flag);
}

ins_t insTpye(uint32_t ins) {
    if (ins == 0) {
        return TERMINATION;
    }
    uint32_t insBits = maskBits(ins, 27, 26);
    if (insBits == 1) {
        return SINGLE_DATA_TRANSFER;
    } else if (insBits == 2) {
        return BRANCH;
    } else {
        uint32_t bitI = maskBits(ins, I_BIT, I_BIT);
        uint32_t bit4 = maskBits(ins, 4, 4);
        uint32_t bit7 = maskBits(ins, 7, 7);
        if (bitI == 0 && bit4 == 1 && bit7 == 1) {
            return MULTIPLY;
        } else {
            return DATA_PROCESSING;
        }
    }
}

cond_t condTpye(uint32_t ins) {
    u_int32_t condBits = maskBits(ins, 31, 28);
    switch (condBits) {
        case 0:
            return EQ;
        case 1:
            return NE;
        case 10:
            return GE;
        case 11:
            return LT;
        case 12:
            return GT;
        case 13:
            return LE;
        case 14:
            return AL;
    }
    return -1;
}

opcode_t opcodeType(uint32_t ins) {
    u_int32_t opcodeBits = maskBits(ins, 24, 21);
    switch (opcodeBits) {
        case 0:
            return AND;
        case 1:
            return EOR;
        case 2:
            return SUB;
        case 3:
            return RSB;
        case 4:
            return ADD;
        case 8:
            return TST;
        case 9:
            return TEQ;
        case 10:
            return CMP;
        case 12:
            return ORR;
        case 13:
            return MOV;
    }
    return -1;
}

shift_t shiftType(uint32_t ins) {
    u_int32_t shiftBits = maskBits(ins, 6, 5);
    switch (shiftBits) {
        case 0:
            return LSL;
        case 1:
            return LSR;
        case 2:
            return ASR;
        case 3:
            return ROR;
    }
    return -1;
}

int checkCond(state_t *state) {
    uint32_t flagBits = maskBits(state->fetched, N_BIT, V_BIT);
    switch (flagBits) {
        case EQ:
            return getZ(state);
        case NE:
            return !getZ(state);
        case GE:
            return getN(state) == getV(state);
        case LT:
            return getN(state) != getV(state);
        case GT:
            return !getZ(state) && getN(state) == getV(state);
        case LE:
            return getZ(state) || getN(state) != getV(state);
        default:
            return 1;
    }
}

u_int32_t shiftData(u_int32_t data, shift_t shift, u_int32_t shiftValue) {
    assert(shiftValue < 32);
    if (shiftValue == 0) {
        return data;
    }
    switch (shift) {
        case LSL:
            data <<= shiftValue;
            break;
        case LSR:
            data >>= shiftValue;
            break;
        case ASR:
            data = (int32_t) data >> shiftValue;
            break;
        case ROR:
            data = (data >> shiftValue) | (data << (BITS_IN_WORD - shiftValue));
            break;
    }
    return data;
}
int shiftCarry(u_int32_t data, shift_t shift, u_int32_t shiftValue) {
    assert(shiftValue < 32);
    if (shiftValue == 0) {
        return 0;
    }
    int carryBit;
    switch (shift) {
        case LSL:
            carryBit = BITS_IN_WORD - shiftValue;
            break;
        case LSR:
        case ASR:
        case ROR:
            carryBit = shiftValue - 1;
            break;
    }
    return maskBits(data, carryBit, carryBit);
}
