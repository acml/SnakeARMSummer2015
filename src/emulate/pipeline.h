#ifndef PIPELINE_H
#define PIPELINE_H

#include "utils.h"

void execute(state_t *state);
void decode(state_t *state);
void fetch(state_t *state);
void incPC(state_t *state);

#endif
