#ifndef PIPELINE_H
#define PIPELINE_H

#include "instructions.h"

void execute(state_t *state);
void decode(state_t *state);
void fetch(state_t *state);
void incPC(state_t *state);
ins_t insType(uint32_t ins);
int checkCond(state_t *state);

#endif
