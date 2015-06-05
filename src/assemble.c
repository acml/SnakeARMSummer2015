#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

/*
 * CONSTANTS:
 *
 * Helper constants for memory and address position
 */
#define MEMORY_SIZE 65536
#define MAX_LINE_LENGTH 512
#define MAX_TOKEN_LENGTH 10
#define BITS_IN_BYTE 8
#define BYTES_IN_WORD 4

#define PC_AHEAD_BYTES 8
#define BRANCH_CONST_POS 25
#define BRANCH_OFFSET_POS 0

/*
 * Constants for representing specific bits positions
 */
#define I_BIT 25
#define S_BIT 20
#define A_BIT 21
#define P_BIT 24
#define U_BIT 23
#define L_BIT 20

/*
 * Constants for register positions in multiply instruction
 */
#define MULTIPLY_RD_POS 16
#define MULTIPLY_RN_POS 12
#define MULTIPLY_CONST_POS 4

/*
 * Constants register and address positions in single data transfer instructions
 */
#define SINGLE_DATA_TRANSFER_CONST_POS 26
#define MAX_OFFSET_LENGTH 7
#define COND_POS 28
#define OPCODE_POS 21
#define RN_POS 16
#define RD_POS 12
#define RS_POS 8
#define RM_POS 0
#define OFFSET_POS 0

/*
 * Constants for shifting bits positions accesss
 */
#define SHIFT_TYPE_POS 5
#define SHIFT_VALUE_POS 7
#define REG_SHIFT_CONST_POS 4
#define IMM_ROTATE_POS 8
#define IMM_VALUE_POS 0



/*
 * Extension constants
 */
#define BX_CONSTANT_POS 4


/*
 * STRUCTURES:
 *
 * TODO:
 */
typedef struct map_elem {
    char *string;
    uint32_t integer;
    struct map_elem *next;
} map_e;

/*
 * TODO:
 */
typedef struct map {
    map_e *head;
} map_t;

typedef struct assemble_maps {
    map_t typeMap;
    map_t condMap;
    map_t opcodeMap;
    map_t shiftMap;
    map_t labelMap;
} maps_t;

/*
 * FUNCTIONS:
 *
 * Map functions for operating with map_elem and map_t structures
 */
map_e *mapAllocElem(void);
void mapFreeElem(map_e *elem);
void mapInit(map_t *m);
void mapDestroy(map_t *m);
void mapPut(map_t *m, char *string, uint32_t integer);
uint32_t mapGet(map_t *m, char *string);

/*
 * Principle functions for performing the assembly using two passes
 */
uint32_t assembly(char **argv, uint8_t *memory);
uint32_t firstPass(FILE *fp, map_t *labelMap);
uint32_t secondPass(FILE *fp, maps_t maps, uint8_t *memory,
        uint32_t programLength);
void storeWord(uint8_t *memory, uint32_t address, uint32_t word);
void writeBinary(char **argv, uint8_t *memory, uint32_t memoryLength);

/*
 *
 */
maps_t initMaps(void);
void destroyMaps(maps_t maps);
map_t initCondMap(void);
map_t initOpcodeMap(void);
map_t initShiftMap(void);
map_t initTypeMap(void);

void preprocessLine(char *buf);
int isLabel(char *buf);
char **tokenizer(char *buf);
void freeTokens(char **tokens);

/*
 * Functions representing subsets of ARM instruction set
 */
uint32_t dataProcessing(char **tokens, maps_t maps);
uint32_t multiply(char **tokens, maps_t maps);
uint32_t singleDataTransfer(char **tokens, maps_t maps, uint8_t *memory,
        uint32_t address, uint32_t *memoryLength);
uint32_t branch(char **tokens, maps_t maps, uint32_t address);

uint32_t getCond(char **tokens, map_t condMap, int offset);


/*
 *Extension instructions
 */
uint32_t extensionIns(char **tokens, maps_t maps, uint32_t address);
uint32_t bx(char **tokens, map_t condMap);
uint32_t pop(char **tokens, map_t condMap);
uint32_t push(char **tokens, map_t condMap);



/*
 * FUNCTION IMPLEMENTATION:
 */

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

    uint32_t memoryLength = assembly(argv, memory);
    writeBinary(argv, memory, memoryLength);

    free(memory);
    return 0;
}

map_e *mapAllocElem(void) {
    map_e *elem = malloc(sizeof(map_e));
    if (elem == NULL) {
        perror("mapAllocElem");
        exit(EXIT_FAILURE);
    }
    return elem;
}

void mapFreeElem(map_e *elem) {
    free(elem->string);
    free(elem);
}

void mapInit(map_t *m) {
    m->head = NULL;
}

void mapDestroy(map_t *m) {
    map_e *elem = m->head;
    while (elem != NULL) {
        map_e *next = elem->next;
        mapFreeElem(elem);
        elem = next;
    }
}

/*
 * Function creates new element with given int value and string representation
 * and insert new element to the head of the map structure
 */
void mapPut(map_t *m, char *string, uint32_t integer) {
    map_e *elem = mapAllocElem();
    elem->string = malloc((strlen(string) + 1) * sizeof(char));
    if (elem->string == NULL) {
        perror("mapPut");
        exit(EXIT_FAILURE);
    }
    strcpy(elem->string, string);
    elem->integer = integer;
    elem->next = m->head;
    m->head = elem;
}

/*
 * Function returns the value in the map structure related with input string
 */
uint32_t mapGet(map_t *m, char *string) {
    map_e *elem = m->head;
    while (strcmp(elem->string, string)) {
        if (elem->next == NULL) {
            printf("%s not found", string);
            exit(EXIT_FAILURE);
        }
        elem = elem->next;
    }
    return elem->integer;
}

/*
 * TODO:
 */
uint32_t assembly(char **argv, uint8_t *memory) {
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Could not open input file.\n");
        exit(EXIT_FAILURE);
    }

    maps_t maps = initMaps();
    uint32_t programLength = firstPass(fp, &maps.labelMap);
    rewind(fp);
    uint32_t memoryLength = secondPass(fp, maps, memory, programLength);
    destroyMaps(maps);

    fclose(fp);
    return memoryLength;
}

/*
 * Function goes through the code checking where there are labels and
 * inserts those labels with their addresses to the map structure
 * and returns end of the code address
 */
uint32_t firstPass(FILE *fp, map_t *labelMap) {
    uint32_t address = 0;
    char buf[MAX_LINE_LENGTH];
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        preprocessLine(buf);
        if (strlen(buf) != 0) {
            if (isLabel(buf)) {
                buf[strlen(buf) - 1] = '\0';
                mapPut(labelMap, buf, address);
            } else {
                address += BYTES_IN_WORD;
            }
        }
    }
    return address;
}

/*
 * Function generates binary encoding splitting line of the code to the tokens
 * and calling given instruction set. After encoding it destroys map structures
 * and returns length of the program.
 */
uint32_t secondPass(FILE *fp, maps_t maps, uint8_t *memory,
        uint32_t programLength) {
    uint32_t address = 0;
    char buf[MAX_LINE_LENGTH];
    /*
     * Split code to the lines ignoring labels
     */
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        preprocessLine(buf);
        if (strlen(buf) != 0 && !isLabel(buf)) {
            char **tokens = tokenizer(buf);
            int route = mapGet(&maps.typeMap, tokens[0]);
            /*
             * Choose instruction set depending on the route value
             */
            //TODO add a fancy enum
            uint32_t ins = 0;
            switch (route) {
                case 0:
                    ins = dataProcessing(tokens, maps);
                    break;
                case 1:
                    ins = multiply(tokens, maps);
                    break;
                case 2:
                    ins = singleDataTransfer(tokens, maps, memory, address,
                            &programLength);
                    break;
                case 3:
                    ins = branch(tokens, maps, address);
                    break;
                case 4:
                    ins = extensionIns(tokens, maps, address);
                    break;
                default:
                    printf("Unsupported instructions type.\n");
                    break;
            }
            storeWord(memory, address, ins);
            freeTokens(tokens);
            address += BYTES_IN_WORD;
        }
    }

    return programLength;
}

/*
 * Function writes memory positions and related data from the given file
 */
void writeBinary(char **argv, uint8_t *memory, uint32_t memoryLength) {
    FILE *fp = fopen(argv[2], "wb");
    if (fp == NULL) {
        printf("Could not open output file.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < memoryLength; i++) {
        fwrite(&memory[i], sizeof(uint8_t), 1, fp);
    }

    fclose(fp);
}

maps_t initMaps(void) {
    maps_t maps;
    maps.typeMap = initTypeMap();
    maps.condMap = initCondMap();
    maps.opcodeMap = initOpcodeMap();
    maps.shiftMap = initShiftMap();
    mapInit(&maps.labelMap);
    return maps;
}

void destroyMaps(maps_t maps) {
    mapDestroy(&maps.typeMap);
    mapDestroy(&maps.condMap);
    mapDestroy(&maps.opcodeMap);
    mapDestroy(&maps.shiftMap);
    mapDestroy(&maps.labelMap);
}

/*
 * TODO:
 */
map_t initTypeMap(void) {
    map_t typeMap;
    mapInit(&typeMap);
    mapPut(&typeMap, "and", 0);
    mapPut(&typeMap, "eor", 0);
    mapPut(&typeMap, "sub", 0);
    mapPut(&typeMap, "rsb", 0);
    mapPut(&typeMap, "add", 0);
    mapPut(&typeMap, "orr", 0);
    mapPut(&typeMap, "mov", 0);
    mapPut(&typeMap, "tst", 0);
    mapPut(&typeMap, "teq", 0);
    mapPut(&typeMap, "cmp", 0);

    mapPut(&typeMap, "mul", 1);
    mapPut(&typeMap, "mla", 1);

    mapPut(&typeMap, "ldr", 2);
    mapPut(&typeMap, "str", 2);

    mapPut(&typeMap, "b", 3);
    mapPut(&typeMap, "ble", 3);
    mapPut(&typeMap, "bgt", 3);
    mapPut(&typeMap, "blt", 3);
    mapPut(&typeMap, "bge", 3);
    mapPut(&typeMap, "bne", 3);
    mapPut(&typeMap, "beq", 3);



    mapPut(&typeMap, "lsl", 0);
    mapPut(&typeMap, "andeq", 0);

//############~Exetension~############
    mapPut(&typeMap, "bx", 4);
    mapPut(&typeMap, "push", 4);
    mapPut(&typeMap, "pop", 4);

    return typeMap;

}

/*
 * TODO:
 */
map_t initCondMap(void) {
    map_t condMap;
    mapInit(&condMap);
    mapPut(&condMap, "eq", 0x0);
    mapPut(&condMap, "ne", 0x1);
    mapPut(&condMap, "ge", 0xa);
    mapPut(&condMap, "lt", 0xb);
    mapPut(&condMap, "gt", 0xc);
    mapPut(&condMap, "le", 0xd);
    mapPut(&condMap, "al", 0xe);
    mapPut(&condMap, "", 0xe);
    return condMap;
}

/*
 * TODO:
 */
map_t initOpcodeMap(void) {
    map_t opcodeMap;
    mapInit(&opcodeMap);
    mapPut(&opcodeMap, "and", 0x0);
    mapPut(&opcodeMap, "eor", 0x1);
    mapPut(&opcodeMap, "sub", 0x2);
    mapPut(&opcodeMap, "rsb", 0x3);
    mapPut(&opcodeMap, "add", 0x4);
    mapPut(&opcodeMap, "orr", 0xc);
    mapPut(&opcodeMap, "mov", 0xd);
    mapPut(&opcodeMap, "tst", 0x8);
    mapPut(&opcodeMap, "teq", 0x9);
    mapPut(&opcodeMap, "cmp", 0xa);
    return opcodeMap;
}

/*
 * TODO:
 */
map_t initShiftMap(void) {
    map_t shiftMap;
    mapInit(&shiftMap);
    mapPut(&shiftMap, "lsl", 0x0);
    mapPut(&shiftMap, "lsr", 0x1);
    mapPut(&shiftMap, "asr", 0x2);
    mapPut(&shiftMap, "ror", 0x3);
    return shiftMap;
}

/*
 * Function stores given word(encoded instruction) to the memory
 */
void storeWord(uint8_t *memory, uint32_t address, uint32_t word) {
    for (int i = 0; i < BYTES_IN_WORD; i++) {
        memory[address + i] = (uint8_t) word;
        word >>= BITS_IN_BYTE;
    }
}

void preprocessLine(char *buf) {
    buf[strlen(buf) - 1] = '\0';
    for (int i = 0; buf[i] != '\0'; i++) {
        if (buf[i] == ';') {
            buf[i] = '\0';
            break;
        }
    }
    if (strlen(buf) != 0) {
        for (int i = strlen(buf) - 1; i >= 0; i--) {
            if (buf[i] == ' ') {
                buf[i] = '\0';
            } else {
                break;
            }
        }
    }
    while (buf[0] == ' ') {
        for (int i = 0; buf[i] != '\0'; i++) {
            buf[i] = buf[i + 1];
        }
    }
}

/*
 * Function returns 1 if the given line of the code includes label
 */
int isLabel(char *buf) {
    return buf[strlen(buf) - 1] == ':';
}

/*
 * Function returns lines of code split to the tokens
 */
char **tokenizer(char *buf) {
    char **tokens = malloc(sizeof(char *) * MAX_TOKEN_LENGTH);
    if (tokens == NULL) {
        perror("tokenizer");
        exit(EXIT_FAILURE);
    }
    char copy[MAX_LINE_LENGTH];
    strcpy(copy, buf);
    char *token = strtok(copy, " ");
    for (int i = 0; i < MAX_TOKEN_LENGTH; i++) {
        if (token != NULL) {
            tokens[i] = malloc((strlen(token) + 1) * sizeof(char));
            strcpy(tokens[i], token);
            token = strtok(NULL, " ,");
        } else {
            tokens[i] = NULL;
        }
    }
    return tokens;
}

void freeTokens(char **tokens) {
    if (tokens != NULL) {
        for (int i = 0; i < MAX_TOKEN_LENGTH; i++) {
            if (tokens[i] != NULL) {
                free(tokens[i]);
            }
        }
        free(tokens);
    }
}

uint32_t branch(char **tokens, maps_t maps, uint32_t address) {   

    uint32_t ins = 0;
    /*
     * work out what cond should be
     */
    uint32_t cond = getCond(tokens, maps.condMap, 1);
    ins |= cond << COND_POS;
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

uint32_t multiply(char **tokens, maps_t maps) {
    uint32_t ins = 0;

    uint32_t cond = getCond(tokens, maps.condMap, 3);
    ins |= cond << COND_POS;
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

    uint32_t cond = getCond(tokens, maps.condMap, 3);
    ins |= cond << COND_POS;
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
            ins |= (uint32_t) offset << OFFSET_POS;
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

uint32_t dataProcessing(char **tokens, maps_t maps) {
    uint32_t ins = 0;

    uint32_t cond = getCond(tokens, maps.condMap, 3);
    ins |= cond << COND_POS;

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

uint32_t getCond(char **tokens, map_t condMap, int offset) {


    uint32_t cond = mapGet(&condMap, tokens[0] + offset);
    tokens[0][offset] = '\0';
    return cond;
}

//##################EXTENSION#################################################

uint32_t extensionIns(char **tokens, maps_t maps, uint32_t address) {
    if(!strcmp(tokens[0], "bx")) {
        return bx(tokens, maps.condMap);
    } else if(!strcmp(tokens[0], "push")) {
        return push(tokens, maps.condMap);
    } else if(!strcmp(tokens[0], "pop")) {
        return pop(tokens, maps.condMap);
    } 
        
        return 0;
    
}


uint32_t bx(char **tokens, map_t condMap) {
    uint32_t ins = 0;
    uint32_t rn = strtol(tokens[1] + 1, NULL, 0);
    const uint32_t constant = 0x12FFF1;
    uint32_t cond = getCond(tokens, condMap, 2);
    printf("%d\n", rn);
    ins |= cond << COND_POS;
    ins |= constant << BX_CONSTANT_POS;
    ins |= rn ;
    return ins;
}

int hasDash(char *token) {
    while(*token != '\0') {
        if (*token == '-') {
            return 1;
        }
        ++token;
    }
    return 0;
}

uint16_t getRegisterList(char **tokens) {
    uint16_t regList =0; // List of registers to be loaded/stored (1 means yes)
    int i = 1; //Start token

    while(1) {
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
                reg = strtol (tokens[i] + 2, NULL, 0);
            } else {
                reg = strtol (tokens[i] + 1, NULL, 0);
            }
            regList |= 1 << reg;
        }
        ++i;
    }
    return regList;
}


uint32_t push(char **tokens, map_t condMap) {
    uint16_t regList = getRegisterList(tokens);
    uint32_t cond = getCond(tokens, condMap, 4);
    uint32_t ins = 0;

    const uint32_t pushConst = 0x92d;
    ins |= pushConst << 16;
    ins |= cond << COND_POS;
    ins |= regList;


    return ins;   
}

uint32_t pop(char **tokens, map_t condMap) {
    uint16_t regList = getRegisterList(tokens);
    uint32_t cond = getCond(tokens, condMap, 4);
    uint32_t ins = 0;

    const uint32_t pushConst = 0x8BD;
    ins |= pushConst << 16;
    ins |= cond << COND_POS;
    ins |= regList;
    return ins;
}

uint32_t blockDataTransfer(char **tokens) {
    //might not be necessary, hardcoded in push pop for now
    return 0;
}


