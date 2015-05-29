#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define INT_BASE 10

#define INT_BASE 10

#define I_BIT 25
#define S_BIT 20
#define A_BIT 21
#define P_BIT 24
#define U_BIT 23
#define L_BIT 20

#define MULTIPLY_CONST 16
#define MULTIPLY_RD 16
#define MULTIPLY_RN 12
#define MULTIPLY_RS 8
#define MULTIPLY_RM 0

#define OFFSET_POS 0
#define COND_POS 28
#define RD_POS 12
#define OPCODE_POS 21
#define RN_POS 16
#define RM_POS 0

#define SHIFT_TYPE_POS 5
#define SHIFT_VALUE_POS 7
#define RS_POS 8

typedef struct linked_map {
    char *string;
    uint32_t integer;
    struct linked_map *next;
} map_t;

void put(map_t *root, char *string, uint32_t integer);
uint32_t get(map_t *root, char *string);

char **tokens(char* str);
uint32_t setBits(uint32_t ins, int value, int pos);
uint32_t setBit(uint32_t ins, int pos);
uint32_t branch(char **tokens, map_t map);
uint32_t multiply(char **tokens);
uint32_t findLabels(map_t *map);
uint32_t dataProcessing(char **tokens);

int main() {
    char str[80] = "mov r0, [1, =3]";
    char **strArrPtr = tokens(str);
    for (int i = 0; i < 10; i++) {
        if (strArrPtr[i] == NULL) {
            break;
        }
        printf("%s\n", strArrPtr[i]);
    }

    return (0);
}

void put(map_t *root, char *string, uint32_t integer) {
    if (root == NULL) {
        root = malloc(sizeof(map_t));
        root->string = string;
        root->integer = integer;
        root->next = NULL;
    }
    while (root->next != NULL) {
        root = root->next;
    }
    map_t *newMap = malloc(sizeof(map_t));
    newMap->string = string;
    newMap->integer = integer;
    newMap->next = NULL;
    root->next = newMap;
}

uint32_t get(map_t *root, char *string) {
    while (strcmp(root->string, string)) {
        root = root->next;
    }
    return root->integer;
}

char** tokens(char* str) {

    const char *s = ", ";
    char **tokArrPtr = malloc(sizeof(char*) * 10);
    char *token;
    token = strtok(str, s);
    int i = 0;

    while (token != NULL) {
        tokArrPtr[i] = token;
        token = strtok(NULL, s);
        i++;
    }
    tokArrPtr[i] = NULL;
    return tokArrPtr;
}

uint32_t setBits(uint32_t ins, int value, int pos) {
    return ins | (value << pos);
}

uint32_t branch(char **tokens, map_t map) {
    uint32_t ins = 0;

    //strcmp returns 0 if there's a match, 1 if no match
    //0 is false, hence !0 indicates there's a match
    //trying to work out what cond should be
    uint32_t cond;
    if (!strcmp(tokens[0], " beq")) {
        cond = 0x0; //0000
    } else if (!strcmp(tokens[0], " bne")) {
        cond = 0x1; //0001
    } else if (!strcmp(tokens[0], " bge")) {
        cond = 0xa; //1010
    } else if (!strcmp(tokens[0], " blt")) {
        cond = 0xb; //1011
    } else if (!strcmp(tokens[0], " bgt")) {
        cond = 0xc; //1100
    } else if (!strcmp(tokens[0], " ble")) {
        cond = 0xd; //1101
    } else {
        //b or bal
        cond = 0xe; //1110
    }

    //for bits 27-24
    uint32_t constant = 0xa;

    //calculate the offset
    uint32_t offset = 0;
    uint32_t next_addr = get(map, tokens[1]);
    uint32_t curr_addr; //somehow get the current address
    //TODO
    //offset = next address - (current address + 8);

    //set cond
    ins = ins | cond << 28;

    //set constant
    ins = ins | constant << 24;

    //set offset
    //ins = ins | offset;
}

uint32_t multiply(char **tokens) {
    uint32_t ins = 0;
    if (!strcmp(tokens[0], "mla")) {
        //mla
        ins = setBits(ins, 1, A_BIT);
        int rn = strtol(tokens[4] + 1, NULL, INT_BASE);
        ins = ins | rn << MULTIPLY_RN;
    }
    int rs = strtol(tokens[3] + 1, NULL, INT_BASE);
    int rm = strtol(tokens[2] + 1, NULL, INT_BASE);
    int rd = strtol(tokens[1] + 1, NULL, INT_BASE);
    ins = ins | rs << MULTIPLY_RS;
    ins = ins | rm << MULTIPLY_RM;
    ins = ins | rd << MULTIPLY_RD;
    int cond = 0xe;
    ins = ins | cond << COND_POS;
    int constField = 9;
    ins = ins | constField << MULTIPLY_CONST;
    return ins;
}

uint32_t findLabels(map_t *map) {
    //TODO implement this

    //returns the address of last instruction
    return 100;    //dummy value
}

uint32_t sDataTrans(char **tokens) {
    uint32_t ins = 0;

    //Check if closing bracket is in tokens[1]
    int postIndexing = 0;
    int i = 0;
    while (postIndexing == 0 && tokens[1][i] != '\0') {
        if (tokens[1][i] == ']') {
            postIndexing = 1;
        }
        i++;
    }
    if (!postIndexing) {
        ins = setBits(ins, 1, P_BIT);
    }
    int rd = strtol(tokens[1] + 1, NULL, 0);
    ins = setBits(ins, rd, RD_POS);
    //TODO : find better way to do this
    if (!strcmp(tokens[0], "ldr")) {
        ins = setBits(ins, 1, L_BIT);
    }
    if (tokens[2][0] == '=') {
        //Immediate value expression
        int immValue = strtol(tokens[2] + 1, NULL, 0);
        if (immValue <= 0xff) {

            ins = setBits(ins, 1, I_BIT);
            ins = setBits(ins, 0xd, OPCODE_POS);
            ins = setBits(ins, immValue, OFFSET_POS);
            return ins;
        }

        //TODO save constants after program terminates
    } else {

        int setU = 0;
        int rn = 0;
        int rm = 0;
        int iBitValue = 0;
        if ((!strcmp(tokens[3], "r15"))) {
            rn = 15;
            // TODO check the rest of the code :P
        } else {
            rn = strtol(tokens[2] + 1, NULL, 0);
        }
        if (tokens[3] == NULL) {
            // set offset to 0
            ins = setBits(ins, 0, OFFSET_POS);
        } else if (tokens[3][0] == '#') {
            // set offset to value of expression
            int expValue = strtol(tokens[3] + 1, NULL, 0);
            ins = setBits(ins, expValue, OFFSET_POS);
            if (expValue < 0) {
                setU = 1;
            }
        } else {
            // if offset of base register is represented by a register
            if (tokens[3][0] == 'r') {
                rm = strtol(tokens[3] + 1, NULL, 0);
            } else {
                rm = strtol(tokens[3] + 2, NULL, 0);
            }
            if (tokens[3][0] != '-') {
                setU = 1;
            }

            // shift cases
            if (tokens[4] != NULL) {
                char *from = tokens[4];
                //char *shiftType = strndup(from, 3); // takes first 3 chars from token line
                char *shiftValue = strdup(from + 4); // takes chars from position 4

                if (shiftValue[0] == '#') {
                    int shiftInt = strtol(shiftValue + 1, NULL, 0);
                    assert(shiftInt < 16);
                    ins = setBits(ins, shiftInt, 7);
                } else {
                    int rs = strtol(shiftValue + 1, NULL, 0);
                    ins = setBits(ins, rs, 8);
                    ins = setBits(ins, 1, 4);
                }
            }
            iBitValue = 1;
        }

        ins = setBits(ins, 0xe, COND_POS);
        ins = setBits(ins, iBitValue, I_BIT);
        ins = setBits(ins, 1, P_BIT);
        ins = setBits(ins, rn, RN_POS);
        ins = setBits(ins, setU, U_BIT);
        ins = setBits(ins, rm, RM_POS);
        //TODO : rewrite shift type
        ins = setBits(ins, 0, 5);
    }

    return ins;
}

uint32_t dataProcessing(char **tokens) {
    uint32_t ins = 0;
    ins |= 0xe << COND_POS;
    uint32_t opcode = 0;
    uint32_t rd = 0;
    uint32_t rn = 0;
    int isS = 0;
    int op2 = 3;
    if (!strcmp(tokens[0], "and")) {
        opcode = 0x0;
        rd = strtol(tokens[1] + 1, NULL, 0);
        rn = strtol(tokens[2] + 1, NULL, 0);
    } else if (!strcmp(tokens[0], "eor")) {
        opcode = 0x1;
        rd = strtol(tokens[1] + 1, NULL, 0);
        rn = strtol(tokens[2] + 1, NULL, 0);
    } else if (!strcmp(tokens[0], "sub")) {
        opcode = 0x2;
        rd = strtol(tokens[1] + 1, NULL, 0);
        rn = strtol(tokens[2] + 1, NULL, 0);
    } else if (!strcmp(tokens[0], "rsb")) {
        opcode = 0x3;
        rd = strtol(tokens[1] + 1, NULL, 0);
        rn = strtol(tokens[2] + 1, NULL, 0);
    } else if (!strcmp(tokens[0], "add")) {
        opcode = 0x4;
        rd = strtol(tokens[1] + 1, NULL, 0);
        rn = strtol(tokens[2] + 1, NULL, 0);
    } else if (!strcmp(tokens[0], "orr")) {
        opcode = 0xc;
        rd = strtol(tokens[1] + 1, NULL, 0);
        rn = strtol(tokens[2] + 1, NULL, 0);
    } else if (!strcmp(tokens[0], "mov")) {
        opcode = 0xd;
        rd = strtol(tokens[1] + 1, NULL, 0);
        op2 = 2;
    } else if (!strcmp(tokens[0], "tst")) {
        opcode = 0x8;
        rn = strtol(tokens[1] + 1, NULL, 0);
        isS = 1;
        op2 = 2;
    } else if (!strcmp(tokens[0], "teq")) {
        opcode = 0x9;
        rn = strtol(tokens[1] + 1, NULL, 0);
        isS = 1;
        op2 = 2;
    } else if (!strcmp(tokens[0], "cmp")) {
        opcode = 0xa;
        rn = strtol(tokens[1] + 1, NULL, 0);
        isS = 1;
        op2 = 2;
    }
    ins |= opcode << OPCODE_POS;
    ins |= rd << RD_POS;
    ins |= rn << RN_POS;
    ins |= isS << S_BIT;
    if (tokens[op2][0] == '#') {
        //TODO: need shift function
    } else {
        uint32_t rm = strtol(tokens[op2] + 1, NULL, 0);
        ins |= rm << RM_POS;
        uint32_t shift = 0;
        if (!strcmp(tokens[op2 + 1], "lsl")) {
            shift = 0;
        } else if (!strcmp(tokens[op2 + 1], "lsr")) {
            shift = 1;
        } else if (!strcmp(tokens[op2 + 1], "asr")) {
            shift = 2;
        } else if (!strcmp(tokens[op2 + 1], "ror")) {
            shift = 3;
        }
        ins |= shift << SHIFT_TYPE_POS;
        if (tokens[op2 + 2][0] == '#') {
            uint32_t shiftValue = strtol(tokens[op2 + 2] + 1, NULL, 0);
            ins |= shiftValue << SHIFT_VALUE_POS;
        } else {
            uint32_t rs = strtol(tokens[op2 + 2] + 1, NULL, 0);
            ins |= rs << RS_POS;
        }
    }
    return ins;
}
