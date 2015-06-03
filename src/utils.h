#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "structs.h"

state_t *newState(void);
void delState(state_t *state);
int readBinary(state_t *state, int argc, char **argv);
int outputState(state_t *state);
void printRegister(state_t *state, int reg);

uint32_t maskBits(uint32_t data, int upper, int lower);
uint32_t getFLag(state_t *state, int flag);
void setFlag(state_t *state, int val, int flag);

#endif
