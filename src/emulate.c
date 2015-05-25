#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <endian.h>
#include <limits.h>

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
#define PC_AHEAD_BYTES 8

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
    uint32_t rd;
    uint32_t rn;
    uint32_t rs;
    uint32_t rm;
    cond_t cond;
    opcode_t opcode;
    shift_t shift;
    uint32_t shiftValue;
    int isRegShiftValue;
    uint32_t immValue;
    uint32_t branchOffset;
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
    uint8_t *memory;
    int isDecoded;
    int isFetched;
    int isTermainated;
} state_t;

typedef struct shift_output {
    uint32_t data;
    int carry;
} shift_o;

typedef struct alu_output {
    uint32_t data;
    int carry;
} alu_o;

state_t *newState(void);
void delState(state_t *state);
int readBinary(state_t *state, int argc, char **argv);
int writeState(state_t *state, char **argv);
void printRegister(state_t *state, FILE *fp, int reg);

void execute(state_t *state);
void decode(state_t *state);
void fetch(state_t *state);
void incPC(state_t *state);

uint32_t maskBits(uint32_t data, int upper, int lower);
uint32_t getFLag(state_t *state, int flag);
void setFlag(state_t *state, int val, int flag);

ins_t insTpye(uint32_t ins);
cond_t condTpye(uint32_t ins);
opcode_t opcodeType(uint32_t ins);
shift_t shiftType(uint32_t ins);

int checkCond(state_t *state);
uint32_t shiftData(uint32_t data, shift_t shift, uint32_t shiftValue);
int shiftCarry(uint32_t data, shift_t shift, uint32_t shiftValue);
shift_o shiftedReg(state_t *state);

void dataProcessing(state_t *state);
void multiply(state_t *state);
void singleDataTransfer(state_t *state);
void branch(state_t *state);

uint32_t dataProcessing_and(state_t *state, uint32_t operand2);
uint32_t dataProcessing_eor(state_t *state, uint32_t operand2);
alu_o dataProcessing_sub(state_t *state, uint32_t operand2);
alu_o dataProcessing_rsb(state_t *state, uint32_t operand2);
alu_o dataProcessing_add(state_t *state, uint32_t operand2);
uint32_t dataProcessing_tst(state_t *state, uint32_t operand2);
uint32_t dataProcessing_teq(state_t *state, uint32_t operand2);
alu_o dataProcessing_cmp(state_t *state, uint32_t operand2);
uint32_t dataProcessing_orr(state_t *state, uint32_t operand2);
uint32_t dataProcessing_mov(state_t *state, uint32_t operand2);

uint32_t offestAddress(uint32_t address, int isU, uint32_t offset);
void ldr(state_t *state, uint32_t address);
void str(state_t *state, uint32_t address);

uint32_t bytesToWord(uint8_t *bytes);

int main(int argc, char **argv) {
    state_t *state = newState();
    if (state == NULL) {
        return EXIT_FAILURE;
    }

    if (!readBinary(state, argc, argv)) {
        return EXIT_FAILURE;
    }

    while (!state->isTermainated) {
        execute(state);
        if (!state->isTermainated) {
            decode(state);
            fetch(state);
            incPC(state);
        }
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

//    uint32_t *word = malloc(sizeof(uint32_t));
//    if (word == NULL) {
//        return 0;
//    }
//    int i = 0;
//    while (fread(word, sizeof(uint32_t), 1, fp)) {
//        state->memory[i] = be32toh(*word);
//        i++;
//    }
//    free(word);

    fread(state->memory, sizeof(uint8_t), MEMORY_SIZE, fp);

    fclose(fp);
    return 1;
}

//int writeState(state_t *state, char **argv) {
//    FILE *fp = fopen(strcat(argv[1], ".out"), "w");
//    if (fp == NULL) {
//        printf("Could not open output file.\n");
//        return 0;
//    }
//
//    fprintf(fp, "Registers:\n");
//    for (int i = 0; i < REGISTERS_COUNT; i++) {
//        if (i != SP_REG && i != LR_REG) {
//            printRegister(state, fp, i);
//        }
//    }
//
//    fprintf(fp, "Non-zero memory:\n");
//    uint32_t *word = (uint32_t *)state->memory;
//    for (int i = 0; i < MEMORY_SIZE / BYTES_IN_WORD; i++) {
//        if (word[i] != 0) {
//            fprintf(fp, "0x%08x: 0x%08x\n", i * 4, be32toh(word[i]));
//        }
//    }
//
//    fclose(fp);
//    return 1;
//}

int writeState(state_t *state, char **argv) {
    FILE *fp = fopen(strcat(argv[1], ".out"), "w");
    if (fp == NULL) {
        printf("Could not open output file.\n");
        return 0;
    }
    fp = stdout;
    fprintf(fp, "Registers:\n");
    for (int i = 0; i < REGISTERS_COUNT; i++) {
        if (i != SP_REG && i != LR_REG) {
            printRegister(state, fp, i);
        }
    }

    fprintf(fp, "Non-zero memory:\n");
    uint32_t *word = (uint32_t *) state->memory;
    for (int i = 0; i < MEMORY_SIZE / BYTES_IN_WORD; i++) {
        if (word[i] != 0) {
            fprintf(fp, "0x%08x: 0x%08x\n", i * 4, be32toh(word[i]));
        }
    }

    fclose(fp);
    return 1;
}

void printRegister(state_t *state, FILE *fp, int reg) {
    if (reg == PC_REG) {
        fprintf(fp, "PC  :");
    } else if (reg == CPSR_REG) {
        fprintf(fp, "CPSR:");
    } else {
        fprintf(fp, "$%-3d:", reg);
    }
    fprintf(fp, "% 11d (0x%08x)\n", state->registers[reg],
            state->registers[reg]);
}

void execute(state_t *state) {
    if (!state->isDecoded) {
        return;
    }

    decoded_t *decoded = state->decoded;
    if (decoded->ins == TERMINATION) {
        state->isTermainated = 1;
        return;
    }

    if (!checkCond(state)) {
        return;
    }

    switch (decoded->ins) {
        case DATA_PROCESSING:
            dataProcessing(state);
            break;
        case MULTIPLY:
            multiply(state);
            break;
        case SINGLE_DATA_TRANSFER:
            singleDataTransfer(state);
            break;
        case BRANCH:
            branch(state);
            break;
        default:
            break;
    }
}

void decode(state_t *state) {
    if (!state->isFetched) {
        state->isDecoded = 0;
        return;
    }

    decoded_t *decoded = state->decoded;
    uint32_t ins = state->fetched;

    decoded->ins = insTpye(ins);

    decoded->rd = maskBits(ins, 15, 12);
    decoded->rn = maskBits(ins, 19, 16);
    if (decoded->ins == MULTIPLY) {
        uint32_t tmp = decoded->rd;
        decoded->rd = decoded->rn;
        decoded->rn = tmp;
    }
    decoded->rs = maskBits(ins, 11, 8);
    decoded->rm = maskBits(ins, 3, 0);

    decoded->cond = condTpye(ins);
    decoded->opcode = opcodeType(ins);
    decoded->shift = shiftType(ins);
    decoded->shiftValue = maskBits(ins, 11, 7);
    decoded->isRegShiftValue = maskBits(ins, 4, 4);
    if (decoded->ins == DATA_PROCESSING) {
        uint32_t data = maskBits(ins, 7, 0);
        uint32_t shiftValue = maskBits(ins, 11, 8) * 2;
        decoded->immValue = shiftData(data, ROR, shiftValue);
    } else {
        decoded->immValue = maskBits(ins, 11, 0);
    }
    uint32_t data = maskBits(ins, 23, 0);
    decoded->branchOffset = shiftData(shiftData(data, LSL, 8), ASR, 6);

    decoded->isI = maskBits(ins, I_BIT, I_BIT);
    decoded->isS = maskBits(ins, S_BIT, S_BIT);
    decoded->isA = maskBits(ins, A_BIT, A_BIT);
    decoded->isP = maskBits(ins, P_BIT, P_BIT);
    decoded->isU = maskBits(ins, U_BIT, U_BIT);
    decoded->isL = maskBits(ins, L_BIT, L_BIT);

    state->isDecoded = 1;
}

void fetch(state_t *state) {
    uint32_t *word = (uint32_t *) state->memory;
    state->fetched = word[state->registers[PC_REG] / BYTES_IN_WORD];
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

uint32_t getFLag(state_t *state, int flag) {
    return maskBits(state->registers[CPSR_REG], flag, flag);
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
    uint32_t condBits = maskBits(ins, 31, 28);
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
    uint32_t opcodeBits = maskBits(ins, 24, 21);
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
    uint32_t shiftBits = maskBits(ins, 6, 5);
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
    switch (state->decoded->cond) {
        case EQ:
            return getFLag(state, Z_BIT);
        case NE:
            return !getFLag(state, Z_BIT);
        case GE:
            return getFLag(state, N_BIT) == getFLag(state, V_BIT);
        case LT:
            return getFLag(state, N_BIT) != getFLag(state, V_BIT);
        case GT:
            return !getFLag(state, Z_BIT)
                    && getFLag(state, N_BIT) == getFLag(state, V_BIT);
        case LE:
            return getFLag(state, Z_BIT)
                    || getFLag(state, N_BIT) != getFLag(state, V_BIT);
        default:
            return 1;
    }
}

uint32_t shiftData(uint32_t data, shift_t shift, uint32_t shiftValue) {
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

int shiftCarry(uint32_t data, shift_t shift, uint32_t shiftValue) {
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

shift_o shiftedReg(state_t *state) {
    decoded_t *decoded = state->decoded;

    uint32_t shiftValue = decoded->shiftValue;
    if (decoded->isRegShiftValue) {
        shiftValue = maskBits(state->registers[decoded->rs], 7, 0);
    }
    shift_o output;
    output.data = shiftData(state->registers[decoded->rm], decoded->shift,
            shiftValue);
    output.carry = shiftCarry(state->registers[decoded->rm], decoded->shift,
            shiftValue);
    return output;
}

// Returns the value of operand2
// if bit I is set, the operand2 is an immediate value rotated right by given
// number of places.
// if bit I is cleared, the operand2 is specified by a register rm. It is either
// rotated by a constant value(if bit4 is cleared) or rotated by a least
// significant byte of rs

void dataProcessing(state_t *state) {
    uint32_t operand2;
    int carry = 0;
    if (state->decoded->isI) {
        operand2 = state->decoded->immValue;
    } else {
        shift_o output = shiftedReg(state);
        operand2 = output.data;
        carry = output.carry;
    }
    uint32_t result;
    alu_o output;
    switch (state->decoded->opcode) {
        case AND:
            result = dataProcessing_and(state, operand2);
            break;
        case EOR:
            result = dataProcessing_eor(state, operand2);
            break;
        case SUB:
            output = dataProcessing_sub(state, operand2);
            result = output.data;
            carry = output.carry;
            break;
        case RSB:
            output = dataProcessing_rsb(state, operand2);
            result = output.data;
            carry = output.carry;
            break;
        case ADD:
            output = dataProcessing_add(state, operand2);
            result = output.data;
            carry = output.carry;
            break;
        case TST:
            result = dataProcessing_tst(state, operand2);
            break;
        case TEQ:
            result = dataProcessing_teq(state, operand2);
            break;
        case CMP:
            output = dataProcessing_cmp(state, operand2);
            result = output.data;
            carry = output.carry;
            break;
        case ORR:
            result = dataProcessing_orr(state, operand2);
            break;
        case MOV:
            result = dataProcessing_mov(state, operand2);
            break;
    }

    switch (state->decoded->opcode) {
        case TST:
        case TEQ:
        case CMP:
            break;
        default:
            state->registers[state->decoded->rd] = result;
            break;
    }

    if (state->decoded->isS) {
        setFlag(state, carry, C_BIT);
        if (result == 0) {
            setFlag(state, 1, Z_BIT);
        } else {
            setFlag(state, 0, Z_BIT);
        }
        setFlag(state, maskBits(result, TOP_BIT, TOP_BIT), N_BIT);
    }
}

// AND rd = operand2 AND rn
uint32_t dataProcessing_and(state_t *state, uint32_t operand2) {
    uint32_t result = state->registers[state->decoded->rn] & operand2;
    return result;
}

// XOR rd = operand2 XOR rn
uint32_t dataProcessing_eor(state_t *state, uint32_t operand2) {
    uint32_t result = state->registers[state->decoded->rn] ^ operand2;
    return result;
}

// Substraction rd = operand2 - rn, if rn > operand2, carry generated
alu_o dataProcessing_sub(state_t *state, uint32_t operand2) {
    alu_o result;
    result.data = (int32_t) state->registers[state->decoded->rn]
            - (int32_t) operand2;
    result.carry = state->registers[state->decoded->rn] >= operand2;
    return result;
}

// Substraction rd = operand2 - rn, if rn > operand2, carry generated
alu_o dataProcessing_rsb(state_t *state, uint32_t operand2) {
    alu_o result;
    result.data = (int32_t) operand2
            - (int32_t) state->registers[state->decoded->rn];
    result.carry = operand2 >= state->registers[state->decoded->rn];
    return result;
}

// Additiotion rd = rn + operand2, unsigned overflow sets carry flag
alu_o dataProcessing_add(state_t *state, uint32_t operand2) {
    alu_o result;
    result.data = (int32_t) state->registers[state->decoded->rn]
            + (int32_t) operand2;
    result.carry = state->registers[state->decoded->rn] > UINT32_MAX - operand2;
    return result;
}

// Test AND on operand2 and rn, result not saved
uint32_t dataProcessing_tst(state_t *state, uint32_t operand2) {
    uint32_t result = state->registers[state->decoded->rn] & operand2;
    return result;
}

// Test XOR on operand2 and rn, result not saved
uint32_t dataProcessing_teq(state_t *state, uint32_t operand2) {
    uint32_t result = state->registers[state->decoded->rn] ^ operand2;
    return result;
}

// Compare rn and operand2, if operand2 if bigger, carry flag is set
alu_o dataProcessing_cmp(state_t *state, uint32_t operand2) {
    alu_o result;
    result.data = (int32_t) state->registers[state->decoded->rn]
            - (int32_t) operand2;
    result.carry = state->registers[state->decoded->rn] >= operand2;
    return result;
}

// Bitwise or on operand2 and rn, result saved in rd
uint32_t dataProcessing_orr(state_t *state, uint32_t operand2) {
    uint32_t result = state->registers[state->decoded->rn] | operand2;
    return result;
}

// Moves the operand2 to register rd
uint32_t dataProcessing_mov(state_t *state, uint32_t operand2) {
    uint32_t result = operand2;
    return result;
}

void multiply(state_t *state) {
    decoded_t *decoded = state->decoded;

    uint32_t result = state->registers[decoded->rs]
            * state->registers[decoded->rm];
    if (decoded->isA) {
        result += state->registers[decoded->rn];
    }
    state->registers[state->decoded->rd] = result;

    if (decoded->isS) {
        if (result == 0) {
            setFlag(state, 1, Z_BIT);
        }
        setFlag(state, maskBits(result, TOP_BIT, TOP_BIT), N_BIT);
    }
}

// Transfers data depending in the conditions
void singleDataTransfer(state_t *state) {
    decoded_t *decoded = state->decoded;

    assert(decoded->rm != PC_REG && decoded->rd != PC_REG);
    assert(decoded->isP || decoded->rm != decoded->rn);

    uint32_t offset;
    if (decoded->isI) {
        offset = shiftedReg(state).data;
    } else {
        offset = decoded->immValue;
    }
    uint32_t address = state->registers[decoded->rn];
    if (decoded->isP) {
        address = offestAddress(address, decoded->isU, offset);
    }
    if (decoded->rn == PC_REG) {
        address += PC_AHEAD_BYTES;
    }

    if (decoded->isL) {
        ldr(state, address);
    } else {
        str(state, address);
    }

    if (!decoded->isP) {
        state->registers[decoded->rn] += offset;
    }
}

uint32_t offestAddress(uint32_t address, int isU, uint32_t offset) {
    if (isU) {
        return address + offset;
    } else {
        return address - offset;
    }
}

// Loads data from memory to register
void ldr(state_t *state, uint32_t address) {
    state->registers[state->decoded->rd] = bytesToWord(&state->memory[address]);
}

// Stores data from register to memory
void str(state_t *state, uint32_t address) {
    uint32_t *word = (uint32_t *) &state->memory[address];
    *word = state->registers[state->decoded->rd];
}

void branch(state_t *state) {
    state->registers[PC_REG] += state->decoded->branchOffset;
}

uint32_t bytesToWord(uint8_t *bytes) {
    return bytes[0] | bytes[1] << 8 | bytes[2] << 16 | bytes[3] << 24;
}

