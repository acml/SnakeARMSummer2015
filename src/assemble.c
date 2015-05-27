#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

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



typedef struct linked_map {
    char *string;
    uint32_t integer;
    struct linked_map *next;
} map_t;

void put(map_t *root, char *string, uint32_t integer);
uint32_t get(map_t *root, char *string);

char **tokens(char* str);
uint32_t setBits(uint32_t ins, int value, int pos);
uint32_t multiply(char **tokens);
uint32_t findLabels(map_t *map);

int main() {
   char str[80] = "mov r0, [1, =3]";
   char  **strArrPtr = tokens(str);
   for(int i = 0 ; i < 10; i++) {
   	if (strArrPtr[i] == NULL) {
   		break;
   	}
   	printf("%s\n", strArrPtr[i]);
   }

   return(0);
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

char **tokens(char* str) {


	const char *s = ", ";
	char **tokArrPtr = malloc(sizeof(char*)* 10);
	char *token;
	token = strtok(str, s);
	int  i = 0;

	while( token != NULL) {
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


uint32_t multiply(char **tokens) {
	uint32_t ins  = 0;
	if (!strcmp(tokens[0], "mla")) {
		//mla
		ins = setBits(ins, 1, A_BIT);
		int rn =  strtol(tokens[4] + 1, NULL, INT_BASE);
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
	return 100;//dummy value
}


uint32_t sDataTrans(char **tokens) {
	uint32_t ins = 0;

	//Check if closing bracket is in tokens[1]
	int postIndexing = 0;
	int i = 0;
	while(postIndexing == 0 && tokens[1][i] != '\0'){
		if(tokens[1][i] == ']') {
			postIndexing = 1;
		}
		i++;
	}
	if(!postIndexing) {
		ins = setBits(ins, 1, P_BIT);
	}
	int rd = strtol(tokens[1] + 1, NULL, 0);
	ins = setBits(ins, rd, RD_POS);
	//TODO : find better way to do this
	if (!strcmp(tokens[0], "ldr")) {
		ins = setBits(ins, 1, L_BIT);
	}
	if (tokens[2][0] == '='){
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
		if((!strcmp(tokens[3], "r15"))) {
			rn = 15;
		// TODO check the rest of the code :P
		} else {
		    rn = strtol(tokens[2] + 1, NULL, 0);
		}
		if(tokens[3] == NULL) {
			// set offset to 0
			ins = setBits(ins, 0, OFFSET_POS);
		} else if(tokens[3][0] == '#') {
			// set offset to value of expression 
			int expValue = strtol(tokens[3] + 1, NULL, 0);
			ins = setBits(ins, expValue, OFFSET_POS);
			if(expValue < 0) {
				setU = 1;
			}
		} else {
			// if offset of base register is represented by a register
			if(tokens[3][0] == 'r') {
				rm = strtol(tokens[3] + 1, NULL, 0);
			} else {
				rm = strtol(tokens[3] + 2, NULL, 0);
			}
			if(tokens[3][0] != '-') {
				setU = 1;
			}
			
			// shift cases
			if(tokens[4] != NULL) {
				char *from = tokens[4];
				//char *shiftType = strndup(from, 3); // takes first 3 chars from token line
				char *shiftValue = strdup(from + 4); // takes chars from position 4

				if(shiftValue[0] == '#') {
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