#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>
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
#define IMM_ROTATE_POS 8

#define MEMORY_SIZE 65536

typedef struct linked_map {
    char *string;
    void *value;
    struct linked_map *next;
} map_t;

void put(map_t *root, char *string, void* value);
void* get(map_t *root, char *string);

char **tokens(char* str);
uint32_t setBits(uint32_t ins, int value, int pos);
uint32_t setBit(uint32_t ins, int pos);
void branch(char **tokens, map_t *map, uint32_t curr_addr, 
        uint8_t *memory, uint32_t constsAdress);
void multiply(char **tokens, map_t *map, uint32_t curr_addr, 
        uint8_t *memory, uint32_t constsAdress);

void dataProcessing(char **tokens, map_t *map, uint32_t curr_addr, 
        uint8_t *memory, uint32_t constsAdress);
void sDataTrans(char **tokens, map_t *map, uint32_t curr_addr, 
        uint8_t *memory, uint32_t constsAdress);
uint32_t firstPass(FILE *fp, map_t *map);
uint32_t secondPass(FILE *fp, map_t *labelsMap, map_t *funcMap, uint32_t length, uint8_t *memory);
int isLabel(char *buf);
void functionMap(map_t *map);


int main(void) {
    FILE *fp;
    fp = fopen("emulate.c", "r"); 
    map_t *funcMap = malloc(sizeof(map_t));
    map_t *labelsMap = malloc(sizeof(map_t));
    uint8_t *memory = malloc(MEMORY_SIZE);
    memset(memory, 0, MEMORY_SIZE);
    uint32_t size = firstPass(fp, labelsMap);
    functionMap(funcMap);
    secondPass(fp, labelsMap, funcMap, size, memory);
    free(labelsMap);
    free(funcMap);
    fclose(fp);

    return 0;



}


int isLabel(char *buf) {
    while(*buf != '\0') {
        if(*buf == ':') {
            return 1;
        }
        ++buf;
    }
    return 0;
}

uint32_t firstPass(FILE *fp, map_t *map) {
    char buf[512];
    int address = 0;
    
    while (fgets (buf, sizeof(buf), fp)) {
        if(isLabel(buf)) {
            put(map, buf, &address);
        } else {
            address += 4;
        }
    }
    
    if (ferror(fp)) {
        fprintf(stderr,"Oops, error reading file\n");
        abort();
    }

    return address;

}

uint32_t secondPass(FILE *fp, map_t *labelsMap, map_t *funcMap, uint32_t length, uint8_t *memory) {
    char buf[512];
    uint32_t address = 0;
    uint32_t constsAddress = length;
    while (fgets (buf, sizeof(buf), fp)) {
        if(!isLabel(buf)) {
            
            char** tok = tokens(buf);
            void *(*func)(char **tokens, map_t *map, uint32_t curr_addr, 
        uint8_t *memory, uint32_t constsAdress) = get(funcMap, tok[0]);
            func(tok, labelsMap, address, memory, constsAddress);
            address += 4;
        } 
    }

    if (ferror(fp)) {
        fprintf(stderr,"Oops, error reading file\n");
        abort();
    }
    return 0;
}


void functionMap(map_t *map) {
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




}

void put(map_t *root, char *string, void *value) {
    if (root == NULL) {
        root = malloc(sizeof(map_t));
        root->string = string;
        root->value = value;
        root->next = NULL;
    }
    while (root->next != NULL) {
        root = root->next;
    }
    map_t *newMap = malloc(sizeof(map_t));
    newMap->string = string;
    newMap->value = value;
    newMap->next = NULL;
    root->next = newMap;
}

void* get(map_t *root, char *string) {
    while (strcmp(root->string, string)) {
        root = root->next;
    }
    return root->value;
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


//uint32_t sDataTrans(char **tokens, uint32_t *constAdress)
void sDataTrans(char **tokens, map_t *map, uint32_t curr_addr, 
        uint8_t *memory, uint32_t constsAdress) {
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
            return;
        } else {
            //TODO save constants after program terminates
        }

        
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
                char *shiftType = strndup(from, 3); // takes first 3 chars from token line
                char *shiftValue = strdup(from + 4); // takes chars from position 4
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

                setBits(ins, shiftBits, 5);
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

        ins = setBits(ins, 0, 5);
    }


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
