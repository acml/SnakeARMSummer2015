#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "definitions.h"
#include "map.h"
#include "maps.h"
#include "binarywriter.h"
#include "instructions.h"

uint32_t setCond(uint32_t ins, char **tokens, map_t condMap);
int hasDash(char *token);
uint16_t getRegisterList(char **tokens);

uint32_t dataProcessing(char **tokens, maps_t maps) {
    uint32_t ins = 0;

    ins = setCond(ins, tokens, maps.condMap);

    if (!strcmp(tokens[0], "lsl")) {
        tokens[4] = tokens[2];
        tokens[3] = tokens[0];
        tokens[0] = malloc((strlen("mov") + 1) * sizeof(char));
        strcpy(tokens[0], "mov");
        tokens[2] = malloc((strlen(tokens[1]) + 1) * sizeof(char));
        strcpy(tokens[2], tokens[1]);
    }

    uint32_t opcode = mapGet(&maps.opcodeMap, tokens[0]);
    ins |= opcode << OPCODE_POS;

    int isS = 0;
    int op2 = 3;
    uint32_t rd = strtol(tokens[1] + 1, NULL, 0);
    uint32_t rn = strtol(tokens[2] + 1, NULL, 0);

    if (!strcmp(tokens[0], "mov")) {
        rn = 0;
        op2 = 2;
    } else if (!strcmp(tokens[0], "tst") || !strcmp(tokens[0], "teq")
            || !strcmp(tokens[0], "cmp")) {
        rn = rd;
        rd = 0;
        isS = 1;
        op2 = 2;
    }
    ins |= rd << RD_POS;
    ins |= rn << RN_POS;
    ins |= isS << S_BIT;

    if (tokens[op2][0] == '#') {
        ins |= 1 << I_BIT;
        uint32_t immValue = strtol(tokens[op2] + 1, NULL, 0);
        uint32_t immRotate = 0;
        int isRepresentable = 0;
        while (!isRepresentable && immRotate <= 30) {
            if (immValue <= 0xFF) {
                isRepresentable = 1;
            } else {
                immValue = (immValue << 2) | (immValue >> 30);
                immRotate += 2;
            }
        }
        if (!isRepresentable) {
            printf("Error: numeric constant not representable.\n");
        } else {
            ins |= immValue << IMM_VALUE_POS;
            ins |= (immRotate / 2) << IMM_ROTATE_POS;
        }
    } else {
        uint32_t rm = strtol(tokens[op2] + 1, NULL, 0);
        ins |= rm << RM_POS;
        if (tokens[op2 + 1] != NULL) {
            uint32_t shift = mapGet(&maps.shiftMap, tokens[op2 + 1]);
            ins |= shift << SHIFT_TYPE_POS;
            if (tokens[op2 + 2][0] == '#') {
                uint32_t shiftValue = strtol(tokens[op2 + 2] + 1, NULL, 0);
                ins |= shiftValue << SHIFT_VALUE_POS;
            } else {
                uint32_t rs = strtol(tokens[op2 + 2] + 1, NULL, 0);
                ins |= rs << RS_POS;
                ins |= 0x1 << REG_SHIFT_CONST_POS;
            }
        }
    }
    return ins;
}

uint32_t multiply(char **tokens, maps_t maps) {
    uint32_t ins = 0;

    ins = setCond(ins, tokens, maps.condMap);
    /*
     * In case of mla function: set bit A and Rn register
     */
    if (!strcmp(tokens[0], "mla")) {
        //mla
        ins |= 1 << A_BIT;
        uint32_t rn = strtol(tokens[4] + 1, NULL, 0);
        ins |= rn << MULTIPLY_RN_POS;
    }
    /*
     * Get values of registers
     */
    uint32_t rd = strtol(tokens[1] + 1, NULL, 0);
    uint32_t rm = strtol(tokens[2] + 1, NULL, 0);
    uint32_t rs = strtol(tokens[3] + 1, NULL, 0);
    ins |= rd << MULTIPLY_RD_POS;
    ins |= rm << RM_POS;
    ins |= rs << RS_POS;
    /*
     * Set constant value 9 starting at position 4
     */
    ins |= 0x9 << MULTIPLY_CONST_POS;
    return ins;
}

uint32_t singleDataTransfer(char **tokens, maps_t maps, uint8_t *memory,
        uint32_t address, uint32_t *memoryLength) {
    /*
     * Put value of expression to the end of assembled program
     * and uses its address with pc register to represent base register
     * and calculated offset
     */
    if (tokens[2][0] == '=') {
        uint32_t immValue = strtol(tokens[2] + 1, NULL, 0);
        if (immValue <= 0xff) {
            tokens[0][0] = 'm';
            tokens[0][1] = 'o';
            tokens[0][2] = 'v';
            tokens[2][0] = '#';
            return dataProcessing(tokens, maps);
        } else {
            storeWord(memory, *memoryLength, immValue);
            uint32_t offset = *memoryLength - address - PC_AHEAD_BYTES;
            *memoryLength += BYTES_IN_WORD;
            free(tokens[2]);
            tokens[2] = malloc((strlen("[r15") + 1) * sizeof(char));
            strcpy(tokens[2], "[r15");
            tokens[3] = malloc(MAX_OFFSET_LENGTH * sizeof(char));
            sprintf(tokens[3], "#%d]", offset);
        }
    }
    uint32_t ins = 0;

    ins = setCond(ins, tokens, maps.condMap);
    /*
     * Set type if the instruction
     */
    if (!strcmp(tokens[0], "ldr")) {
        ins |= 1 << L_BIT;
    }
    /*
     * Set Rd, Rn value
     */
    uint32_t rd = strtol(tokens[1] + 1, NULL, 0);
    uint32_t rn = strtol(tokens[2] + 2, NULL, 0);
    ins |= rd << RD_POS;
    ins |= rn << RN_POS;
    /*
     * Set pre/postIndexing bit
     */
    if (tokens[2][strlen(tokens[2]) - 1] != ']' || tokens[3] == NULL) {
        ins |= 1 << P_BIT;
    }

    int isU = 1;
    if (tokens[3] != NULL) {
        if (tokens[3][0] == '#') {
            /*
             * Offset is represented by numerical value
             */
            int offset = strtol(tokens[3] + 1, NULL, 0);
            if (offset < 0) {
                offset = -offset;
                isU = 0;
            }
            ins |= (uint32_t) offset << SINGLE_DATA_TRANSFER_OFFSET_POS;
        } else {
            ins |= 1 << I_BIT;
            /*
             * Offset represented as a register
             */
            if (tokens[3][0] == '-') {
                isU = 0;
            }
            uint32_t rm;
            if (tokens[3][0] == 'r') {
                rm = strtol(tokens[3] + 1, NULL, 0);
            } else {
                rm = strtol(tokens[3] + 2, NULL, 0);
            }
            ins |= rm << RM_POS;
            /*
             * Register is shifted
             */
            if (tokens[4] != NULL) {
                uint32_t shift = mapGet(&maps.shiftMap, tokens[4]);
                ins |= shift << SHIFT_TYPE_POS;
                if (tokens[5][0] == '#') {
                    uint32_t shiftValue = strtol(tokens[5] + 1, NULL, 0);
                    ins |= shiftValue << SHIFT_VALUE_POS;
                } else {
                    uint32_t rs = strtol(tokens[5] + 1, NULL, 0);
                    ins |= rs << RS_POS;
                    ins |= 0x1 << REG_SHIFT_CONST_POS;
                }
            }
        }
    }
    ins |= isU << U_BIT;

    ins |= 0x1 << SINGLE_DATA_TRANSFER_CONST_POS;
    return ins;
}

uint32_t branch(char **tokens, maps_t maps, uint32_t address) {
    uint32_t ins = 0;
    /*
     * work out what cond should be
     */
    ins = setCond(ins, tokens, maps.condMap);

    if (tokens[0][1] == 'l') {
        ins |= 1 << BRANCH_L_BIT;
    }
    /*
     * calculate the offset
     */
    uint32_t lableAddress = mapGet(&maps.labelMap, tokens[1]);
    uint32_t offset = ((lableAddress - address - PC_AHEAD_BYTES) << 6) >> 8;
    ins |= offset << BRANCH_OFFSET_POS;
    /*
     * for bits 27-24
     */
    ins |= 0x5 << BRANCH_CONST_POS;
    return ins;
}

uint32_t setCond(uint32_t ins, char **tokens, map_t condMap) {
    int typeLength = getTypeLength(tokens);
    uint32_t cond = mapGet(&condMap, tokens[0] + typeLength);
    tokens[0][typeLength] = '\0';
    return ins |= cond << COND_POS;
}

int getTypeLength(char **tokens) {
    int typeLength = 3;
    if (tokens[0][0] == 'b') {
        if (tokens[0][1] == 'x'
                || (tokens[0][1] == 'l' && strlen(tokens[0]) != 3)) {
            typeLength = 2;
        } else {
            typeLength = 1;
        }
    } else if (tokens[0][0] == 'p' && tokens[0][1] == 'u') {
        typeLength = 4;
    }
    return typeLength;
}

//##################EXTENSION#################################################

uint32_t bx(char **tokens, maps_t maps) {
    uint32_t ins = 0;
    ins = setCond(ins, tokens, maps.condMap);
    uint32_t rn = strtol(tokens[1] + 1, NULL, 0);
    ins |= rn;
    ins |= 0x12fff1 << BX_CONST_POS;
    return ins;
}

int hasDash(char *token) {
    while (*token != '\0') {
        if (*token == '-') {
            return 1;
        }
        ++token;
    }
    return 0;
}

uint16_t getRegisterList(char **tokens) {
    uint16_t regList = 0; // List of registers to be loaded/stored (1 means yes)
    int i = 1; //Start token

    while (1) {
        if (tokens[i] == NULL) {
            break;
        }

        if (hasDash(tokens[i])) {
            char *delim = "{}- ";
            char *tok = strtok(tokens[i], delim);

            int start = strtol(tok + 1, NULL, 0);
            tok = strtok(NULL, delim);
            printf("%s\n", tok);
            int end = strtol(tok + 1, NULL, 0);
            for (int j = start; j <= end; j++) {
                regList |= (1 << j);
            }
        } else {
            int reg;
            if (tokens[i][0] == '{') {
                reg = strtol(tokens[i] + 2, NULL, 0);
            } else {
                reg = strtol(tokens[i] + 1, NULL, 0);
            }
            regList |= 1 << reg;
        }
        ++i;
    }
    return regList;
}

uint32_t pushPop(char **tokens, maps_t maps) {
    uint32_t ins = 0;
    ins = setCond(ins, tokens, maps.condMap);
    uint32_t regList = getRegisterList(tokens);
    ins |= regList;
    if (!strcmp(tokens[0], "push")) {
        ins |= 0x92d << PUSH_POP_CONST_POS;
    } else {
        ins |= 0x8bd << PUSH_POP_CONST_POS;
    }
    return ins;
}
