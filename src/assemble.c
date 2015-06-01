#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>
#include <string.h>
#include <assert.h>

#define MAX_LINE_LENGTH 512
#define BYTES_IN_WORD 4

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
#define IMM_ROTATE_POS 8

#define MEMORY_SIZE 65536

typedef struct map_elem {
    char *string;
    int integer;
    struct map_elem *next;
} map_e;

typedef struct map {
    map_e *head;
} map_t;

map_e *mapAllocElem(void);
void mapFreeElem(map_e *elem);
void mapInit(map_t *m);
void mapPut(map_t *m, char *string, int integer);

uint32_t assembly(char **argv, uint8_t *memory);
uint32_t firstPass(FILE *fp, map_t *labelMap);
uint32_t secondPass(FILE *fp, map_t *labelMap, uint32_t programLength, uint8_t *memory);

map_t initOpcodeMap(void);
map_t initCondMap(void);
map_t initShiftMap(void);

char **tokens(char* str);
uint32_t setBits(uint32_t ins, int value, int pos);
//uint32_t setBit(uint32_t ins, int pos);

void branch(char **tokens, map_t *map, uint32_t curr_addr,
        uint8_t *memory, uint32_t constsAdress);
void multiply(char **tokens, map_t *map, uint32_t curr_addr,
        uint8_t *memory, uint32_t constsAdress);

void dataProcessing(char **tokens, map_t *map, uint32_t curr_addr,
        uint8_t *memory, uint32_t constsAdress);
void sDataTrans(char **tokens, map_t *map, uint32_t curr_addr,
        uint8_t *memory, uint32_t constsAdress);
int isLabel(char *buf);
//void functionMap(map_t *map);

// single data transfer helper functions
uint32_t setIndexing(char **tokens, uint32_t ins);
uint32_t setImmValue(uint32_t ins, char **tokens);
uint32_t setRnValue(char **tokens, uint32_t ins) ;
uint32_t setShiftType(uint32_t ins, char *shiftType);
uint32_t setShiftValue(uint32_t ins, char* shiftValue);

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Input file and/or output file not specified.\n");
        exit(EXIT_FAILURE);
    }

    uint8_t *memory = malloc(MEMORY_SIZE);
    if (memory == NULL) {
        perror("memory");
        exit(EXIT_FAILURE);
    }
    memset(memory, 0, MEMORY_SIZE);

//    functionMap(funcMap);

    return 0;
}

map_e *mapAllocElem(void) {
    map_e *elem = malloc(sizeof(map_e));
    if (elem == NULL) {
        perror("newListElem");
        exit(EXIT_FAILURE);
    }
    return elem;
}

void mapFreeElem(map_e *elem) {
    free(elem);
}

void mapInit(map_t *m) {
    m->head = NULL;
}

void mapPut(map_t *m, char *string, int integer) {
    map_e *elem = mapAllocElem();
    elem->string = string;
    elem->integer = integer;
    elem->next = m->head;
    m->head = elem;
}

int mapGet(map_t *m, char *string) {
    map_e *elem = m->head;
    while (strcmp(elem->string, string)) {
        elem = elem->next;
    }
    return elem->integer;
}

uint32_t assembly(char **argv, uint8_t *memory) {
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Could not open input file.\n");
        exit(EXIT_FAILURE);
    }

    map_t labelMap;
    mapInit(&labelMap);

    uint32_t programLength = firstPass(fp, &labelMap);
    uint32_t totalLength = secondPass(fp, &labelMap, programLength, memory);

    fclose(fp);
    return totalLength;
}

uint32_t firstPass(FILE *fp, map_t *labelMap) {
    uint32_t address = 0;
    char buf[MAX_LINE_LENGTH];
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if(isLabel(buf)) {
            mapPut(labelMap, buf, address);
        } else {
            address += BYTES_IN_WORD;
        }
    }
    return address;
}

int isLabel(char *buf) {
    return buf[strlen(buf) - 1] == ':';
}

map_t initOpcodeMap(void) {
    map_t opcodeMap;
    mapInit(opcodeMap);
    mapPut(opcodeMap, "and", 0x0);
    mapPut(opcodeMap, "eor", 0x1);
    mapPut(opcodeMap, "sub", 0x2);
    mapPut(opcodeMap, "rsb", 0x3);
    mapPut(opcodeMap, "add", 0x4);
    mapPut(opcodeMap, "orr", 0xc);
    mapPut(opcodeMap, "mov", 0xd);
    mapPut(opcodeMap, "tst", 0x8);
    mapPut(opcodeMap, "teq", 0x9);
    mapPut(opcodeMap, "cmp", 0xa);
    return opcodeMap;
}

map_t initCondMap(void) {
    map_t condMap;
    mapInit(condMap);
    mapPut(condMap, "eq", 0x0);
    mapPut(condMap, "ne", 0x1);
    mapPut(condMap, "ge", 0xa);
    mapPut(condMap, "lt", 0xb);
    mapPut(condMap, "gt", 0xc);
    mapPut(condMap, "le", 0xd);
    mapPut(condMap, "al", 0xe);
    mapPut(condMap, "", 0xe);
    return condMap;
}

map_t initShiftMap(void) {
    map_t shiftMap;
    mapInit(shiftMap);
    mapPut(shiftMap, "lsl", 0x0);
    mapPut(shiftMap, "lsr", 0x1);
    mapPut(shiftMap, "asr", 0x2);
    mapPut(shiftMap, "ror", 0x3);
    return shiftMap;
}

uint32_t secondPass(FILE *fp, map_t *labelMap, uint32_t programLength, uint8_t *memory) {
    map_t opcodeMap = initOpcodeMap();
    map_t condMap = initCondMap();
    map_t shiftMap = initShiftMap();

    uint32_t address = 0;
    uint32_t totalLength = programLength;
    char buf[MAX_LINE_LENGTH];
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if(!isLabel(buf)) {
            //TODO
            address += BYTES_IN_WORD;
        }
    }
    return totalLength;
}


/*void functionMap(map_t *map) {
    put(map, "add", &dataProcessing);
    put(map, "sub", &dataProcessing);
    put(map, "rsb", &dataProcessing);
    put(map, "and", &dataProcessing);
    put(map, "eor", &dataProcessing);
    put(map, "orr", &dataProcessing);
    put(map, "mov", &dataProcessing);
    put(map, "tst", &dataProcessing);
    put(map, "teq", &dataProcessing);
    put(map, "cmp", &dataProcessing);
    put(map, "andeq", &dataProcessing);

    put(map, "mul", &multiply);
    put(map, "mla", &multiply);

    put(map, "ldr", &sDataTrans);
    put(map, "str", &sDataTrans);

    put(map, "beq", &branch);
    put(map, "bne", &branch);
    put(map, "bge", &branch);
    put(map, "blt", &branch);
    put(map, "bgt", &branch);
    put(map, "ble", &branch);
    put(map, "b", &branch);
}*/

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

void branch(char **tokens, map_t *map, uint32_t curr_addr,
        uint8_t *memory, uint32_t constsAdress) {
 /*   uint32_t ins = 0;

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
*/
}

void multiply(char **tokens, map_t *map, uint32_t curr_addr,
        uint8_t *memory, uint32_t constsAdress) {
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

}

void binWriter(uint32_t ins, char *fileName) {
    FILE *fp;
    fp = fopen(fileName, "wb");
    fwrite(&ins, 4, 1, fp);
    fclose(fp);
}

//uint32_t sDataTrans(char **tokens, uint32_t *constAdress)
void sDataTrans(char **tokens, map_t *map, uint32_t curr_addr,
        uint8_t *memory, uint32_t constsAdress) {
    uint32_t ins = 0;

    ins = setIndexing(tokens, ins);
    // set value of Rd
    int rd = strtol(tokens[1] + 1, NULL, 0);
    ins = setBits(ins, rd, RD_POS);
    // set instruction type
    int isLoad = 0;
    if (!strcmp(tokens[0], "ldr")) {
        ins = setBits(ins, 1, L_BIT);
        isLoad = 1;
    }
    // address as a numeric constant form
    if(isLoad == 1) {
        //Immediate value expression
        if (tokens[2][0] == '=') {
            ins = setImmValue(ins, tokens);
        }
    } else {
        int setU = 0;
        ins = setRnValue(tokens, ins);
        int rm = 0;
        int iBitValue = 0;

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
                char *shiftType = strndup(from, 3); // takes first 3 chars from token line
                char *shiftValue = strdup(from + 4); // takes chars from position 4
                ins = setShiftType(ins, shiftType);
                ins = setShiftValue(ins, shiftValue);
            }
            iBitValue = 1;
        }
        ins = setBits(ins, 0xe, COND_POS);
        ins = setBits(ins, iBitValue, I_BIT);
        ins = setBits(ins, setU, U_BIT);
        ins = setBits(ins, rm, RM_POS);
        ins = setBits(ins, 0, 5);
    }
}

uint32_t setIndexing(char **tokens, uint32_t ins) {
    int postIndexing = 0;
    int i = 0;
    //Check if closing bracket is in tokens[1]
    while (postIndexing == 0 && tokens[1][i] != '\0') {
        if (tokens[1][i] == ']') {
            postIndexing = 1;
        }
        i++;
    }
    if (!postIndexing) {
        ins = setBits(ins, 1, P_BIT);
    }
    return ins;
}

uint32_t setImmValue(uint32_t ins, char **tokens) {
    int immValue = strtol(tokens[2] + 1, NULL, 0);
    if (immValue <= 0xff) {
        ins = setBits(ins, 1, I_BIT);
        ins = setBits(ins, 0xd, OPCODE_POS);
        ins = setBits(ins, immValue, OFFSET_POS);
        return ins;
    } else {
        //TODO save constants after program terminates
        return 0;
    }
}

uint32_t setRnValue(char **tokens, uint32_t ins) {
    int rn = 0;
    if ((!strcmp(tokens[3], "r15"))) {
        rn = 15;
    } else {
        rn = strtol(tokens[2] + 1, NULL, 0);
    }
    ins = setBits(ins, rn, RN_POS);
    return ins;
}

uint32_t setShiftType(uint32_t ins, char *shiftType) {
    int shiftBits = 0;
    if (!strcmp(shiftType, "lsl")) {
        shiftBits = 0x0; //00
    } else if (!strcmp(shiftType, "lsr")) {
        shiftBits = 0x1; //01
    } else if (!strcmp(shiftType, " asr")) {
        shiftBits = 0x2; //10
    } else if (!strcmp(shiftType, " ror")) {
        shiftBits = 0x3; //11
    }
    ins = setBits(ins, shiftBits, 5);
    return ins;
}

uint32_t setShiftValue(uint32_t ins, char* shiftValue) {
    if (shiftValue[0] == '#') {
        int shiftInt = strtol(shiftValue + 1, NULL, 0);
        assert(shiftInt < 16);
        ins = setBits(ins, shiftInt, 7);
    } else {
        int rs = strtol(shiftValue + 1, NULL, 0);
        ins = setBits(ins, rs, 8);
        ins = setBits(ins, 1, 4);
    }
    return ins;
}

void dataProcessing(char **tokens, map_t *map, uint32_t curr_addr,
        uint8_t *memory, uint32_t constsAdress) {
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
        uint32_t immValue = strtol(tokens[op2] + 1, NULL, 0);
        uint32_t shiftValue = 0;
        int isRepresentable = 0;
        while (!isRepresentable && shiftValue <= 30) {
            if (immValue <= 0xFF) {
                isRepresentable = 1;
            } else {
                immValue = (immValue >> 2) | (immValue << 30);
                shiftValue += 2;
            }
        }
        if (!isRepresentable) {
            printf("Error: not representable");
        } else {
            ins |= immValue;
            ins |= (shiftValue / 2) << IMM_ROTATE_POS;
        }
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
}
