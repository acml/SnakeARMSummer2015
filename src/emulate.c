#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


#define REGISTERS_COUNT 17
#define MEMORY_SIZE 65536

struct State state;
struct State {
    unsigned int registers[17]; //Todo check
    int *memory;
};


unsigned int getN (void);
unsigned int getZ(void);
unsigned int getC(void);
unsigned int getV(void);
unsigned int maskInt(unsigned int data, int upper, int lower);
unsigned int instructionType(unsigned int instruction);
int checkCond(unsigned int instruction);
void initialize(void);

int main(int argc, char **argv) {

    initialize();
    unsigned int testInstruction = 10 << 28;
    state.registers[16] = 0xf << 28;
    unsigned int data = 0x0000000f;
    unsigned int result = maskInt(data, 5, 2);
    printf("CPSR : %u \n", result);

    checkCond(testInstruction);
    return EXIT_SUCCESS;
}



void initialize(void) {
    state.memory = malloc(MEMORY_SIZE * sizeof(int));
    memset(state.registers, 0, REGISTERS_COUNT);
    memset(state.memory, 0, MEMORY_SIZE* sizeof(int));
}

int checkCond(unsigned int instruction) {
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

unsigned int getV(void){
    unsigned int temp = state.registers[16] << 3;
    return temp >> 31;
}

unsigned int getC(void){
    unsigned int temp = state.registers[16] << 2;
    return temp >> 31;
}

unsigned int getZ(void) {
    unsigned int temp = state.registers[16] << 1;
    return temp >> 31;
}

unsigned int getN (void) {

    return state.registers[16] >> 31;
}

unsigned int maskInt(unsigned int data, int upper, int lower) {
    assert(upper >= lower);
    data = data << (31 - upper);
    data = data >> (31 - (upper - lower));
    return data;
}

unsigned int instructionType(unsigned int instruction) {
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