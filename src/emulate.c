#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define REGISTERS_COUNT 17
#define MEMORY_SIZE 65536
#define CPSR 16
#define N_POS 31
#define Z_POS 30
#define C_POS 29
#define V_POS 28
#define S_POS 20



struct State state;
struct State {
    uint32_t registers[17]; //Todo check
    uint32_t *memory;
};


uint32_t getN (void);
uint32_t getZ(void);
uint32_t getC(void);
uint32_t getV(void);
uint32_t maskInt(uint32_t data, int upper, int lower);
uint32_t instructionType(uint32_t instruction);
int checkCond(uint32_t instruction);
void initialize(void);
void routeInstruction(uint32_t instruction);
void dproc(uint32_t instruction);
uint32_t dproc_and(uint32_t instruction, uint32_t operand2);

int main(int argc, char **argv) {

    initialize();
    uint32_t testInstruction = 10 << 28;
    state.registers[CPSR] = 0xf << 28;
    uint32_t data = 0x0000000f;
    uint32_t result = maskInt(data, 5, 2);
    
    uint32_t instruction = 0xffffffff;
    
    routeInstruction(instruction);
    
    printf("CPSR : %u \n", result);

    checkCond(testInstruction);
    return EXIT_SUCCESS;
}



void initialize(void) {
    state.memory = malloc(MEMORY_SIZE * sizeof(uint32_t));
    memset(state.registers, 0, REGISTERS_COUNT);
    memset(state.memory, 0, MEMORY_SIZE* sizeof(uint32_t));
}

int checkCond(uint32_t instruction) {
    instruction = instruction >> 28;
    switch (instruction) {
        case 0 :
            return getZ();
        case 1 :
            return (1 - getZ());
        case 10 :
            return getN() == getV();
        case 11 :
            return getN() != getV();
        case 12 :
            return getZ() == 0 && getN() == getV();
        case 13 :
            return getZ() == 1 || getN() != getV();
        case 14 :
            return 1;
    }
    return 0;
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

//TODO fix those
uint32_t getV(void){
    return maskInt(state.registers[CPSR], V_POS, V_POS);
}

uint32_t getC(void){
    return maskInt(state.registers[CPSR], C_POS, C_POS);
}

uint32_t getZ(void) {
    return maskInt(state.registers[CPSR], Z_POS, Z_POS);
}

uint32_t getN (void) {
    return maskInt(state.registers[CPSR], N_POS, N_POS);
}


uint32_t maskInt(uint32_t data, int upper, int lower) {
    assert(upper >= lower);
    data = data << (31 - upper);
    data = data >> (31 - (upper - lower));
    return data;
}

uint32_t instructionType(uint32_t instruction) {
    int selectionBits = maskInt(instruction, 27, 26);
    if (selectionBits == 0) {
        int bit25 = maskInt(instruction, 25, 25);
        if (bit25 == 1) {
            return 0;
        }
        int bit4 = maskInt(instruction, 4, 4);
        int bit7 = maskInt(instruction, 7, 7);
        if (bit4 == 1 && bit7 == 1) {
            return 1;
        } else {
            return 0;
        }
    } else if (selectionBits == 1) {
        return 2;
    } else if (selectionBits == 2) {
        return 3;
    } else {

        printf("something went terribly wrong");
        return -1;
    }

}

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
			//multiply(instruction);
			return;
		case 2:
			//sDataTrans(instruction);
			return;
		case 3:
			//branch(instruction);
			return;
		default:			
			return;
	}
}


void dproc(uint32_t instruction) {
	int opcode = maskInt(instruction, 24, 21);
	uint32_t operand2 = 0; //TODO implement this
	uint32_t result;
	switch(opcode) {
		case 0:
			result = dproc_and(instruction, operand2);
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

uint32_t dproc_and(uint32_t instruction, uint32_t operand2) {
	int rn = maskInt(instruction, 19, 16);
	int rd = maskInt(instruction, 15, 12);
	uint32_t result = state.registers[rn] & operand2;
	state.registers[rd] = result;
	return result;
}
