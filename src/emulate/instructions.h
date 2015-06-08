#ifndef INS_H
#define INS_H

#include "utils.h"

uint32_t shiftData(uint32_t data, shift_t shift, uint32_t shiftValue);
int shiftCarry(uint32_t data, shift_t shift, uint32_t shiftValue);
shift_o shiftReg(state_t *state);

void dataProcessing(state_t *state);
void multiply(state_t *state);
void singleDataTransfer(state_t *state);
void branch(state_t *state);

#endif
