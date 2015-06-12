#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "instructions.h"
#include "binarywriter.h"
#include "definitions.h"

uint32_t setCond(uint32_t ins, char **tokens, map_t condMap);
uint32_t setRegShift(uint32_t ins, char **tokens, map_t shiftMap, int pos);
uint32_t setRegList(uint32_t ins, char **tokens);

uint32_t dataProcessing(char **tokens, maps_t maps) {
    uint32_t ins = 0;

    ins = setCond(ins, tokens, maps.condMap);
    /*
     * Convert lsl Rn,<#expression> to mov Rn,Rn,lsl <#expression>
     */
    if (!strcmp(tokens[0], "lsl")) {
        tokens[4] = tokens[2];
        tokens[3] = tokens[0];
        tokens[0] = malloc((strlen("mov") + 1) * sizeof(char));
        if (tokens[0] == NULL) {
            perror("dataProcessing");
            exit(EXIT_FAILURE);
        }
        strcpy(tokens[0], "mov");
        tokens[2] = malloc((strlen(tokens[1]) + 1) * sizeof(char));
        if (tokens[2] == NULL) {
            perror("dataProcessing");
            exit(EXIT_FAILURE);
        }
        strcpy(tokens[2], tokens[1]);
    }

    uint32_t opcode = mapGet(&maps.opcodeMap, tokens[0]);
    ins |= opcode << OPCODE_POS;

    int isS = 0;
    int op2 = 3;
    uint32_t rd = strtol(tokens[1] + 1, NULL, 0);
    uint32_t rn = strtol(tokens[2] + 1, NULL, 0);
    /*
     * Set rd, rn and S bit according to instruction
     */
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
        /*
         * Op2 is immediate value
         */
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
            exit(EXIT_FAILURE);
        } else {
            ins |= immValue << IMM_VALUE_POS;
            ins |= (immRotate / 2) << IMM_ROTATE_POS;
        }
    } else {
        /*
         * Op2 is register
         */
        uint32_t rm = strtol(tokens[op2] + 1, NULL, 0);
        ins |= rm << RM_POS;
        ins = setRegShift(ins, tokens, maps.shiftMap, op2 + 1);
    }
    return ins;
}

uint32_t multiply(char **tokens, maps_t maps) {
    uint32_t ins = 0;

    ins = setCond(ins, tokens, maps.condMap);
    /*
     * Set A bit and Rn register for mla
     */
    if (!strcmp(tokens[0], "mla")) {
        //mla
        ins |= 1 << A_BIT;
        uint32_t rn = strtol(tokens[4] + 1, NULL, 0);
        ins |= rn << MULTIPLY_RN_POS;
    }
    /*
     * Set values of registers
     */
    uint32_t rd = strtol(tokens[1] + 1, NULL, 0);
    uint32_t rm = strtol(tokens[2] + 1, NULL, 0);
    uint32_t rs = strtol(tokens[3] + 1, NULL, 0);
    ins |= rd << MULTIPLY_RD_POS;
    ins |= rm << RM_POS;
    ins |= rs << RS_POS;
    /*
     * Set instruction constant
     */
    ins |= 0x9 << MULTIPLY_CONST_POS;
    return ins;
}

uint32_t singleDataTransfer(char **tokens, maps_t maps, uint8_t *memory,
        uint32_t address, uint32_t *memoryLength) {
    if (tokens[2][0] == '=') {
        uint32_t immValue = strtol(tokens[2] + 1, NULL, 0);
        if (immValue <= 0xff) {
            /*
             * Use mov instead if expression is less than or equal to 0xff
             */
            tokens[0][0] = 'm';
            tokens[0][1] = 'o';
            tokens[0][2] = 'v';
            tokens[2][0] = '#';
            return dataProcessing(tokens, maps);
        } else {
            /*
             * Convert ldr Rd,<=expression> to ldr Rd,[PC,<#offest>] by
             * put value of expression at the end of assembled program
             * and use its address and PC register to calculated offset
             */
            storeWord(memory, *memoryLength, immValue);
            uint32_t offset = *memoryLength - address - PC_AHEAD_BYTES;
            *memoryLength += BYTES_IN_WORD;
            free(tokens[2]);
            tokens[2] = malloc((strlen("[r15") + 1) * sizeof(char));
            if (tokens[2] == NULL) {
                perror("singleDataTransfer");
                exit(EXIT_FAILURE);
            }
            strcpy(tokens[2], "[r15");
            tokens[3] = malloc(MAX_OFFSET_LENGTH * sizeof(char));
            if (tokens[3] == NULL) {
                perror("singleDataTransfer");
                exit(EXIT_FAILURE);
            }
            sprintf(tokens[3], "#%d]", offset);
        }
    }
    uint32_t ins = 0;

    ins = setCond(ins, tokens, maps.condMap);
    /*
     * Set L bit for ldr
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
            int32_t offset = strtol(tokens[3] + 1, NULL, 0);
            if (offset < 0) {
                offset = -offset;
                isU = 0;
            }
            ins |= (uint32_t) offset << SINGLE_DATA_TRANSFER_OFFSET_POS;
        } else {
            /*
             * Offset is represented by a register
             */
            ins |= 1 << I_BIT;
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
            ins = setRegShift(ins, tokens, maps.shiftMap, 4);
        }
    }
    ins |= isU << U_BIT;
    /*
     * Set instruction constant
     */
    ins |= 0x1 << SINGLE_DATA_TRANSFER_CONST_POS;
    return ins;
}

uint32_t branch(char **tokens, maps_t maps, uint32_t address) {
    uint32_t ins = 0;

    ins = setCond(ins, tokens, maps.condMap);
    /*
     * Set L bit for bl
     */
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
     * Set instruction constant
     */
    ins |= 0x5 << BRANCH_CONST_POS;
    return ins;
}

/*
 * Function sets cond code according to mnemonic and remove the mnemonic
 */
uint32_t setCond(uint32_t ins, char **tokens, map_t condMap) {
    int typeLength = getTypeLength(tokens);
    uint32_t cond = mapGet(&condMap, tokens[0] + typeLength);
    tokens[0][typeLength] = '\0';
    return ins | cond << COND_POS;
}

/*
 * Function sets shift applied to Rm
 */
uint32_t setRegShift(uint32_t ins, char **tokens, map_t shiftMap, int pos) {
    if (tokens[pos] != NULL) {
        uint32_t shift = mapGet(&shiftMap, tokens[pos]);
        ins |= shift << SHIFT_TYPE_POS;
        pos++;
        if (tokens[pos][0] == '#') {
            uint32_t shiftValue = strtol(tokens[pos] + 1, NULL, 0);
            ins |= shiftValue << SHIFT_VALUE_POS;
        } else {
            uint32_t rs = strtol(tokens[pos] + 1, NULL, 0);
            ins |= rs << RS_POS;
            ins |= 0x1 << REG_SHIFT_CONST_POS;
        }
    }
    return ins;
}

/*
 * Function returns instruction length without cond mnemonic
 */
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

uint32_t pushPop(char **tokens, maps_t maps) {
    uint32_t ins = 0;
    ins = setCond(ins, tokens, maps.condMap);
    ins = setRegList(ins, tokens);
    if (!strcmp(tokens[0], "push")) {
        ins |= 0x92d << PUSH_POP_CONST_POS;
    } else {
        ins |= 0x8bd << PUSH_POP_CONST_POS;
    }
    return ins;
}

/*
 * Function sets register list for pop and push instructions
 * 0-15 bits are set corresponding for r0-15
 */
uint32_t setRegList(uint32_t ins, char **tokens) {
    for (int i = 1; tokens[i] != NULL; i++) {
        if (tokens[i][2] == '-') {
            uint32_t startReg = strtol(tokens[i] + 1, NULL, 0);
            uint32_t endReg = strtol(tokens[i] + 4, NULL, 0);
            for (int reg = startReg; reg <= endReg; reg++) {
                ins |= 1 << reg;
            }
        } else {
            uint32_t reg = strtol(tokens[i] + 1, NULL, 0);
            ins |= 1 << reg;
        }
    }
    return ins;
}
