#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>

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

#define REGISTERS_COUNT 17
#define WORD_SIZE 4
#define CPSR 16





#define PC 15

#define N_POS 31
#define Z_POS 30
#define C_POS 29
#define V_POS 28


//Data processing related constants
#define S_POS 20
#define I_POS 25


#define IMM_OPERAND_LOW 0
#define IMM_OPERAND_HIGH 7
#define IMM_ROTATE_LOW 8
#define IMM_ROTATE_HIGH 11

#define REG_RM_LOW 0
#define REG_RM_HIGH 3
#define SHIFT_TYPE_LOW 5
#define SHIFT_TYPE_HIGH 6
#define REG_RS_LOW 8
#define REG_RS_HIGH 11
#define SHIFT_CONST_LOW 7
#define SHIFT_CONST_HIGH 11



typedef struct state {
    uint32_t fetched;
    uint32_t *registers;
    uint32_t *memory;
    int isDecoded;
    int isFetched;
}state_t;

typedef enum {
    DATA_PROCESSING,
    MULTIPLY,
    SINGLE_DATA_TRANSFER,
    BRANCH
}ins_t;

state_t *newState(void);
void delState(state_t *state);
void execut(state_t *state);
void decode(state_t *state);
void fetch(state_t *state);
void incPC(state_t *state);
uint32_t maskBits(uint32_t data, int upper, int lower);
uint32_t getN(state_t *state);
uint32_t getZ(state_t *state);
uint32_t getC(state_t *state);
uint32_t getV(state_t *state);

ins_t insTpye(state_t *state);

int isCondTrue(state_t *state);

void initialize(const char *filename);
void routeInstruction(uint32_t instruction);
void dproc(uint32_t instruction);


//DATA PROCESSING INSTRUCTIONS
uint32_t dproc_and(uint32_t operand2, uint8_t rn, uint8_t rd);
uint32_t dproc_eor(uint32_t operand2, uint8_t rn, uint8_t rd);
uint32_t dproc_sub(uint32_t operand2, uint8_t rn, uint8_t rd);
uint32_t dproc_rsb(uint32_t operand2, uint8_t rn, uint8_t rd);
uint32_t dproc_add(uint32_t operand2, uint8_t rn, uint8_t rd);
uint32_t dproc_tst(uint32_t operand2, uint8_t rn);
uint32_t dproc_teq(uint32_t operand2, uint8_t rn);
uint32_t dproc_cmp(uint32_t operand2, uint8_t rn);
uint32_t dproc_orr(uint32_t operand2, uint8_t rn, uint8_t rd);
uint32_t dproc_mov(uint32_t operand2, uint8_t rd);

//DATA PROCESSING HELPERS
uint32_t dproc_getOperand2(uint32_t instruction);
uint32_t shift(uint8_t type, uint32_t number, uint32_t offset);


void multiply(uint32_t instruction);
void branch(uint32_t instruction);
uint32_t multiply_normal(uint32_t instruction);
uint32_t multiply_acc(uint32_t instruction);

uint8_t checkInput(int argc, char **argv);



int main(int argc, char **argv) {
    if (checkInput(argc, argv) !=  0) {
        return EXIT_FAILURE;
    }

    state_t *state = newState();
    if (state == NULL) {
        return EXIT_FAILURE;
    }

    while(1) {
        execut(state);
        decode(state);
        fetch(state);
        incPC(state);
    }




    initialize(argv[1]);
    printf("0x%08X\n", state.memory[0]);
    //uint32_t testInstruction = 10 << 28;
    //state.registers[CPSR] = 0xf << 28;
    //uint32_t data = 0x0000000f;
    //uint32_t result = maskInt(data, 5, 2);

    //uint32_t instruction = 0xffffffff;

    //routeInstruction(instruction);



    //checkCond(testInstruction);
    return EXIT_SUCCESS;
}

uint8_t checkInput(int argc, char **argv) {
    if (argc <= 1) {
        printf("No input specified.\n");
        return -1;
    }

    if(access( argv[1], F_OK ) == -1) {
        printf("Given input file does not exist.\n");
        return -1;
    }

    if(access( argv[1], R_OK ) == -1 ) {
        printf("The file can't be read.\n");
        return -1;
    }
    return 0;
}

void initialize(const char *filename) {

    state.memory = malloc(MEMORY_SIZE * sizeof(uint32_t));
    FILE *fp = fopen(filename, "r");
    fread(state.memory, WORD_SIZE, MEMORY_SIZE, fp);
    fclose(fp);

    memset(state.registers, 0, REGISTERS_COUNT);

}


void setFlag(int val, int pos) {
	if (val == 0) {
		val = 1 << pos;
		val = ~val;
		state.registers[CPSR] = state.registers[CPSR] & val;
	} else {
		val = 1 << pos;
		state.registers[CPSR] = state.registers[CPSR] | val;
	}
}



// Routes instructions to a correct router
void routeInstruction(uint32_t instruction) {
	if (!checkCond(instruction)) {
		return;
	}
	int type = instructionType(instruction);
	switch(type) {
		case 0:
			dproc(instruction);
			return;
		case 1:
			multiply(instruction);
			return;
		case 2:
			//sDataTrans(instruction);
			return;
		case 3:
			branch(instruction);
			return;
		default:
			return;
	}
}

// Routes the dataProcessing instruction to a correct function
void dproc(uint32_t instruction) {
	int opcode = maskInt(instruction, 24, 21);
	uint32_t operand2 = dproc_getOperand2(instruction);
	uint32_t result;
	uint8_t rn = maskInt(instruction, 19, 16);
	uint8_t rd = maskInt(instruction, 15, 12);
	switch(opcode) {
		case 0:
			result = dproc_and(operand2, rn, rd);
			return;
		case 1:
			result = dproc_eor(operand2, rn, rd);
			return;
		case 2:
			result = dproc_sub(operand2, rn, rd);
			return;
		case 3:
			result = dproc_rsb(operand2, rn, rd);
			return;
		case 4:
			result = dproc_add(operand2, rn, rd);
			return;
		case 8:
			result = dproc_tst(operand2, rn);
			return;
		case 9:
			result = dproc_teq(operand2, rn);
			return;
		case 10:
			result = dproc_cmp(operand2, rn);
			return;
		case 12:
			result = dproc_orr(operand2, rn, rd);
			return;
		case 13:
			result = dproc_mov(operand2, rd);
			return;

		default:

			return;
	}
	int sBit = maskInt(instruction, S_POS, S_POS);
	if (sBit) {
		if (result == 0) {
			setFlag(1, Z_POS);
		} else {
			setFlag(0, Z_POS);
		}
		setFlag(maskInt(result, 31, 31), N_POS);
	}
}

// Rotates the given number right by a given offset
uint32_t rotateRight(uint32_t number, uint32_t offset) {
    return (number >> offset) || (number << (32 - offset));
}


// Returns the value of operand2
// if bit I is set, the operand2 is an immediate value rotated right by given
// number of places.
// if bit I is cleared, the operand2 is specified by a register rm. It is either
// rotated by a constant value(if bit4 is cleared) or rotated by a least
// significant byte of rs
uint32_t dproc_getOperand2(uint32_t instruction) {
    int iBit = maskInt(instruction, I_POS, I_POS);
    uint32_t operand2;
    if (iBit) {
        operand2 = maskInt(instruction, IMM_OPERAND_HIGH, IMM_OPERAND_LOW);
        uint8_t rotate = maskInt(instruction, IMM_ROTATE_HIGH, IMM_ROTATE_LOW);
        operand2 = rotateRight(operand2, rotate * 2);
        return operand2;
    }
    uint8_t rm = maskInt(instruction, REG_RM_HIGH, REG_RM_LOW);

    uint8_t bit4 = maskInt(instruction, 4, 4);
    uint8_t shiftType = maskInt(instruction, SHIFT_TYPE_LOW, SHIFT_TYPE_HIGH);
    uint32_t offset;
    if (bit4) {
        uint8_t rs = maskInt(instruction, REG_RS_HIGH, REG_RS_LOW);
        offset = maskInt(state.registers[rs], 7, 0);
    } else {
        offset = maskInt(instruction, SHIFT_CONST_HIGH, SHIFT_CONST_LOW);
    }
    operand2 = shift(shiftType, state.registers[rm], offset);
    return operand2;
}

// Binary shifter
// Shift type :
//      0 - logic left,
//      1 - logic right,
//      2 - arithmetic right,
//      3 -rotate right
// Bits are always shifted by offset value and the last bit to be discarded
// (rotated) is set to carry flag
uint32_t shift(uint8_t type, uint32_t number, uint32_t offset) {
    if (offset < 1) {
        return number;
    }
    uint32_t result;
    uint8_t carryOut;
    switch (type) {
        case 0:
            carryOut = maskInt(number, 31 - offset, 31 - offset);
            result = number << offset;
            break;
        case 1:

            carryOut = maskInt(number, offset - 1, offset - 1);
            result = number >> offset;
            break;
        case 2:
            carryOut = maskInt(number, offset - 1, offset - 1);
            result = (int32_t) number >> offset;
            break;
        case 3:
            carryOut = maskInt (number, offset - 1, offset - 1);
            result = rotateRight(number, offset);
            break;
    }
    setFlag(carryOut, C_POS);
    return result;
}

// AND rd = operand2 AND rn
uint32_t dproc_and(uint32_t operand2,
		uint8_t rn, uint8_t rd) {

	uint32_t result = state.registers[rn] & operand2;
	state.registers[rd] = result;
	return result;
}


// XOR rd = operand2 XOR rn
uint32_t dproc_eor(uint32_t operand2, uint8_t rn, uint8_t rd) {
    uint32_t result = state.registers[rn] ^ operand2;
    state.registers[rd] = result;
    return result;
}

// Substraction rd = operand2 - rn, if rn > operand2, carry generated
uint32_t dproc_sub(uint32_t operand2, uint8_t rn, uint8_t rd) {
    if ((int32_t) state.registers[rn] < (int32_t)operand2) {
        setFlag(1, C_POS);
    } else {
        setFlag(0, C_POS);
    }
	uint32_t result = (int32_t) state.registers[rn] - (int32_t)operand2;
	state.registers[rd] = result;
	return result;
}

// Substraction rd = operand2 - rn, if rn > operand2, carry generated
uint32_t dproc_rsb(uint32_t operand2, uint8_t rn, uint8_t rd) {
    if ((int32_t)operand2 < (int32_t) state.registers[rn]) {
        setFlag(1, C_POS);
    } else {
        setFlag(0, C_POS);
    }
	uint32_t result = operand2 - (int32_t) state.registers[rn];
	state.registers[rd] = result;
	return result;
}

// Additiotion rd = rn + operand2, unsigned overflow sets carry flag
uint32_t dproc_add(uint32_t operand2, uint8_t rn, uint8_t rd) {
    if ((int32_t) state.registers[rn]  > INT_MAX - (int32_t)operand2) {
        setFlag(1, C_POS);
    } else {
        setFlag(0, C_POS);
    }
	uint32_t result = (int32_t) state.registers[rn] + (int32_t)operand2;
	state.registers[rd] = result;
	return result;
}

// Test AND on operand2 and rn, result not saved
uint32_t dproc_tst(uint32_t operand2, uint8_t rn) {
	uint32_t result = state.registers[rn] & operand2;
	return result;
}


// Test XOR on operand2 and rn, result not saved
uint32_t dproc_teq(uint32_t operand2, uint8_t rn) {
	uint32_t result = state.registers[rn] ^ operand2;
	return result;
}

// Compare rn and operand2, if operand2 if bigger, carry flag is set
uint32_t dproc_cmp(uint32_t operand2, uint8_t rn) {
    if ((int32_t) state.registers[rn] < (int32_t)operand2) {
        setFlag(1, C_POS);
    } else {
        setFlag(0, C_POS);
    }
	uint32_t result = (int32_t) state.registers[rn] - (int32_t)operand2;
	return result;
}

// Bitwise or on operand2 and rn, result saved in rd
uint32_t dproc_orr(uint32_t operand2, uint8_t rn, uint8_t rd) {
	uint32_t result = state.registers[rn] | operand2;
	state.registers[rd] = result;
	return result;
}

// Moves the operand2 to register rd
uint32_t dproc_mov(uint32_t operand2, uint8_t rd) {
	uint32_t result = operand2;
	state.registers[rd] = result;
	return result;
}

void multiply(uint32_t instruction) {
    uint32_t result;
    if (checkCond(instruction) == 1) {
        uint8_t bit21 = maskInt(instruction, 21, 21);
        if (bit21 == 1) {
            result = multiply_acc(instruction);
        } else {
            result = multiply_normal(instruction);
        }

        uint8_t sBit = maskInt(instruction, 20, 20);
        if (sBit == 1) {
            if (result == 0) {
                setFlag(1, Z_POS);
            }
            setFlag(maskInt(result, 31, 31), N_POS);
        }

    }
}

uint32_t multiply_acc(uint32_t instruction) {
    uint8_t rd = maskInt(instruction, 19, 16);
    uint8_t rn = maskInt(instruction, 15, 12);
    uint8_t rs = maskInt(instruction, 11, 8);
    uint8_t rm = maskInt(instruction, 3, 0);
    uint32_t result;

    result = state.registers[rs] * state.registers[rm];
    result += state.registers[rn];
    state.registers[rd] = result;

    return result;
}

uint32_t multiply_normal(uint32_t instruction) {
    uint8_t rd = maskInt(instruction, 19, 16);
    uint8_t rs = maskInt(instruction, 11, 8);
    uint8_t rm = maskInt(instruction, 3, 0);
    uint32_t result;

    result = state.registers[rs] * state.registers[rm];
    state.registers[rd] = result;

    return result;
}

void branch(uint32_t instruction) {
    if (checkCond(instruction) == 1) {
        //get a 24 bit value and shift it left
        uint32_t mask = maskInt(instruction, 23, 0);
        mask = mask << 2;

        //move it upto 31st bit
        //move it back so leading 1s/0s are added (it's signed now)
        int32_t offset = mask << 6;
        offset = offset >> 6;

        state.registers[PC] += mask;
    }
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
    assert(upper >= lower && upper <=31 && lower >=0);
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

uint32_t getC(state_t *state){
    return maskBits(state->registers[CPSR_REG], C_BIT, C_BIT);
}

uint32_t getV(state_t *state){
    return maskBits(state->registers[CPSR_REG], V_BIT, V_BIT);
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

int isCondTrue(state_t *state) {
    uint32_t condBits = maskBits(state->fetched, N_BIT, V_BIT);
    switch(condBits) {
        case 0:
            return getZ(state) == 1;
        case 1:
            return getZ(state) == 0;
        case 10:
            return getN(state) == getV(state);
        case 11:
            return getN(state) != getV(state);
        case 12:
            return getZ(state) == 0 && getN(state) == getV(state);
        case 13:
            return getZ(state) == 1 || getN(state) != getV(state);
        default:
            return 1;
    }
}
