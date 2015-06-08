#ifndef ASSEMBLY_FUNC
#define ASSEMBLY_FUNC
#include "mapStructs.h"
#include "map.h"
/*
 * Principle functions for performing the assembly using two passes
 */
uint32_t assembly(char **argv, uint8_t *memory);
uint32_t firstPass(FILE *fp, map_t *labelMap);
uint32_t secondPass(FILE *fp, maps_t maps, uint8_t *memory,
        uint32_t programLength);
void preprocessLine(char *buf);
int isLabel(char *buf);
char **tokenizer(char *buf);
void freeTokens(char **tokens);
#endif
