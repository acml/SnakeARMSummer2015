#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include "definitions.h"

/*
 * Contains all the decoded information from an instruction
 */
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

/*
 * Holds the complete state of the emulated processor
 */
typedef struct arm_state {
    decoded_t *decoded;
    uint32_t fetched;
    uint32_t *registers;
    uint8_t *memory;
    int isDecoded;
    int isFetched;
    int isTermainated;
} state_t;

/*
 * Holds the output of shifter
 */
typedef struct shift_output {
    uint32_t data;
    int carry;
} shift_o;

state_t *newState(void);
void delState(state_t *state);

uint32_t maskBits(uint32_t data, int upper, int lower);
uint32_t getFLag(state_t *state, int flag);
void setFlag(state_t *state, int val, int flag);

uint32_t shiftData(uint32_t data, shift_t shift, uint32_t shiftValue);
shift_o shiftReg(state_t *state);

#endif
