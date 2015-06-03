#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

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
#define L_BIT 20
#define SP_REG 13
#define LR_REG 14
#define PC_REG 15
#define CPSR_REG 16
#define PC_AHEAD_BYTES 8

typedef enum {
    DATA_PROCESSING,
    MULTIPLY,
    SINGLE_DATA_TRANSFER,
    BRANCH,
    TERMINATION
} ins_t;

typedef enum {
    EQ = 0,
    NE = 1,
    GE = 10,
    LT = 11,
    GT = 12,
    LE = 13,
    AL = 14
} cond_t;

typedef enum {
    AND = 0,
    EOR = 1,
    SUB = 2,
    RSB = 3,
    ADD = 4,
    TST = 8,
    TEQ = 9,
    CMP = 10,
    ORR = 12,
    MOV = 13
} opcode_t;

typedef enum {
    LSL = 0,
    LSR = 1,
    ASR = 2,
    ROR = 3
} shift_t;

typedef struct arm_decoded {
    ins_t ins;
    int isRegShiftValue;
    int isI;
    int isS;
    int isA;
    int isP;
    int isU;
    int isL;
    uint32_t rd;
    uint32_t rn;
    uint32_t rs;
    uint32_t rm;
    cond_t cond;
    opcode_t opcode;
    shift_t shift;
    uint32_t shiftValue;
    uint32_t immValue;
    uint32_t branchOffset;
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

state_t *newState(void);
void delState(state_t *state);
int readBinary(state_t *state, int argc, char **argv);
int outputState(state_t *state);
void printRegister(state_t *state, int reg);

void execute(state_t *state);
void decode(state_t *state);
void fetch(state_t *state);
void incPC(state_t *state);

uint32_t maskBits(uint32_t data, int upper, int lower);
uint32_t getFLag(state_t *state, int flag);
void setFlag(state_t *state, int val, int flag);

ins_t insTpye(uint32_t ins);
int checkCond(state_t *state);
uint32_t shiftData(uint32_t data, shift_t shift, uint32_t shiftValue);
int shiftCarry(uint32_t data, shift_t shift, uint32_t shiftValue);
shift_o shiftReg(state_t *state);

void dataProcessing(state_t *state);
void multiply(state_t *state);
void singleDataTransfer(state_t *state);
void branch(state_t *state);

//entry point of emulator
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

    if (!outputState(state)) {
        return EXIT_FAILURE;
    }

    delState(state);
    return EXIT_SUCCESS;
}

//
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
    int i = 0;
    while (fread(buffer, sizeof(uint8_t), 1, fp) == 1) {
        state->memory[i] = *buffer;
        i++;
    }
    free(buffer);

    fclose(fp);
    return 1;
}

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

    memset(decoded, 0, sizeof(decoded_t));
    uint32_t instruction = state->fetched;

    decoded->ins = insTpye(instruction);

    decoded->isRegShiftValue = maskBits(instruction, 4, 4);
    decoded->isI = maskBits(instruction, I_BIT, I_BIT);
    decoded->isS = maskBits(instruction, S_BIT, S_BIT);
    decoded->isA = maskBits(instruction, A_BIT, A_BIT);
    decoded->isP = maskBits(instruction, P_BIT, P_BIT);
    decoded->isU = maskBits(instruction, U_BIT, U_BIT);
    decoded->isL = maskBits(instruction, L_BIT, L_BIT);

    if (decoded->ins != BRANCH) {
        decoded->rd = maskBits(instruction, 15, 12);
        decoded->rn = maskBits(instruction, 19, 16);
        if (decoded->ins == MULTIPLY) {
            uint32_t tmp = decoded->rd;
            decoded->rd = decoded->rn;
            decoded->rn = tmp;
            decoded->rs = maskBits(instruction, 11, 8);
            decoded->rm = maskBits(instruction, 3, 0);
        } else if ((decoded->ins == DATA_PROCESSING && !decoded->isI) ||
                   (decoded->ins == SINGLE_DATA_TRANSFER && decoded->isI)) {
            decoded->rm = maskBits(instruction, 3, 0);
            if (decoded->isRegShiftValue) {
                decoded->rs = maskBits(instruction, 11, 8);
            }
        }
    }

    decoded->cond = maskBits(instruction, 31, 28);
    decoded->opcode = maskBits(instruction, 24, 21);
    decoded->shift = maskBits(instruction, 6, 5);
    decoded->shiftValue = maskBits(instruction, 11, 7);
    if (decoded->ins == DATA_PROCESSING) {
        uint32_t data = maskBits(instruction, 7, 0);
        uint32_t shiftValue = maskBits(instruction, 11, 8) * 2;
        decoded->immValue = shiftData(data, ROR, shiftValue);
    } else {
        decoded->immValue = maskBits(instruction, 11, 0);
    }
    uint32_t data = maskBits(instruction, 23, 0);
    decoded->branchOffset = shiftData(shiftData(data, LSL, 8), ASR, 6);

    state->isDecoded = 1;
}

void fetch(state_t *state) {
    state->fetched = ((uint32_t *) state->memory)[state->registers[PC_REG]
            / BYTES_IN_WORD];
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
    assert(val == 0 || val == 1);
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
    } else if (maskBits(ins, I_BIT, I_BIT) == 0 && maskBits(ins, 7, 4) == 9) {
        return MULTIPLY;
    } else {
        return DATA_PROCESSING;
    }
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
    int carryBit = 0;
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

shift_o shiftReg(state_t *state) {
    decoded_t *decoded = state->decoded;

    uint32_t shiftValue = decoded->shiftValue;
    if (decoded->isRegShiftValue) {
        shiftValue = maskBits(state->registers[decoded->rs], 7, 0);
    }
    uint32_t data = state->registers[decoded->rm];

    shift_o output;
    output.data = shiftData(data, decoded->shift, shiftValue);
    output.carry = shiftCarry(data, decoded->shift, shiftValue);
    return output;
}

// Returns the value of operand2
// if bit I is set, the operand2 is an immediate value rotated right by given
// number of places.
// if bit I is cleared, the operand2 is specified by a register rm. It is either
// rotated by a constant value(if bit4 is cleared) or rotated by a least
// significant byte of rs

void dataProcessing(state_t *state) {
    decoded_t *decoded = state->decoded;

    assert(decoded->rd != PC_REG && decoded->rn != PC_REG &&
           decoded->rs != PC_REG && decoded->rm != PC_REG);

    uint32_t operand2;
    int carry = 0;
    if (decoded->isI) {
        operand2 = decoded->immValue;
    } else {
        shift_o output = shiftReg(state);
        operand2 = output.data;
        carry = output.carry;
    }

    uint32_t result = 0;
    switch (decoded->opcode) {
        case AND:
        case TST:
            result = state->registers[decoded->rn] & operand2;
            break;
        case EOR:
        case TEQ:
            result = state->registers[decoded->rn] ^ operand2;
            break;
        case SUB:
        case CMP:
            result = (int32_t) state->registers[decoded->rn]
                    - (int32_t) operand2;
            carry = state->registers[decoded->rn] >= operand2;
            break;
        case RSB:
            result = (int32_t) operand2
                    - (int32_t) state->registers[decoded->rn];
            carry = operand2 >= state->registers[decoded->rn];
            break;
        case ADD:
            result = (int32_t) state->registers[decoded->rn]
                    + (int32_t) operand2;
            carry = state->registers[decoded->rn] > UINT32_MAX - operand2;
            break;
        case ORR:
            result = state->registers[decoded->rn] | operand2;
            break;
        case MOV:
            result = operand2;
            break;
    }

    switch (decoded->opcode) {
        case TST:
        case TEQ:
        case CMP:
            break;
        default:
            state->registers[decoded->rd] = result;
            break;
    }

    if (decoded->isS) {
        setFlag(state, carry, C_BIT);
        if (result == 0) {
            setFlag(state, 1, Z_BIT);
        } else {
            setFlag(state, 0, Z_BIT);
        }
        setFlag(state, maskBits(result, TOP_BIT, TOP_BIT), N_BIT);
    }
}

void multiply(state_t *state) {
    decoded_t *decoded = state->decoded;

    assert(decoded->rd != PC_REG && decoded->rn != PC_REG &&
           decoded->rs != PC_REG && decoded->rm != PC_REG);
    assert(decoded->rd != decoded->rm);

    uint32_t result = state->registers[decoded->rs]
            * state->registers[decoded->rm];
    if (decoded->isA) {
        result += state->registers[decoded->rn];
    }
    state->registers[decoded->rd] = result;

    if (decoded->isS) {
        if (result == 0) {
            setFlag(state, 1, Z_BIT);
        } else {
            setFlag(state, 0, Z_BIT);
        }
        setFlag(state, maskBits(result, TOP_BIT, TOP_BIT), N_BIT);
    }
}

int checkPinAddresses(uint32_t address) {
	if(address == 0x20200028) {
	    printf("PIN OFF\n");
	    return 1;
    }
    if(address == 0x2020001C) {
        printf("PIN ON\n");
        return 1;
    }
    if(address == 0x20200000) {

        printf("One GPIO pin from 0 to 9 has been accessed\n");
        return 1;
    }
    if (address == 0x20200004) {
        printf("One GPIO pin from 10 to 19 has been accessed\n");
        return 1;
    }
    if (address == 0x20200008) {
        printf("One GPIO pin from 20 to 29 has been accessed\n");
        return 1;
    }
    return 0;
}

// Transfers data depending in the conditions
void singleDataTransfer(state_t *state) {
    decoded_t *decoded = state->decoded;

    assert(decoded->rm != PC_REG && decoded->rd != PC_REG);
    assert(decoded->isP || decoded->rm != decoded->rn);

    uint32_t offset;
    if (decoded->isI) {
        offset = shiftReg(state).data;
    } else {
        offset = decoded->immValue;
    }
    if (!decoded->isU) {
        offset = -offset;
    }

    uint32_t address = state->registers[decoded->rn];
    if (decoded->isP) {
        address += offset;
    }

    if (address > MEMORY_SIZE) {
		if (checkPinAddresses(address)) {
			if(decoded->isL) {
				state->registers[decoded->rd] = address;
			}
		} else {
            printf("Error: Out of bounds memory access at address 0x%08x\n",
                address);
        }
        return;
    }

    if (decoded->isL) {
        state->registers[decoded->rd] = *((uint32_t *) &state->memory[address]);
    } else {
        *((uint32_t *) &state->memory[address]) = state->registers[decoded->rd];
    }

    if (!decoded->isP) {
        address += offset;
        if (decoded->rn == PC_REG) {
            address += PC_AHEAD_BYTES;
        }
        state->registers[decoded->rn] = address;
    }

}

void branch(state_t *state) {
    state->registers[PC_REG] += state->decoded->branchOffset;
    state->isDecoded = 0;
    state->isFetched = 0;
}
