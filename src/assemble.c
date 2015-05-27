#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define INT_BASE 10

#define I_BIT 25
#define S_BIT 20
#define A_BIT 21
#define P_BIT 24
#define U_BIT 23
#define L_BIT 20

#define COND 28


#define MULTIPLY_CONST 16
#define MULTIPLY_RD 16
#define MULTIPLY_RN 12
#define MULTIPLY_RS 8
#define MULTIPLY_RM 0


typedef struct linked_map {
    char *string;
    uint32_t integer;
    struct linked_map *next;
} map_t;

void put(map_t *root, char *string, uint32_t integer);
uint32_t get(map_t *root, char *string);

char **tokens(char* str);
uint32_t setBit(uint32_t ins, int pos);
uint32_t multiply(char **tokens);



int main()
{
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

uint32_t setBit(uint32_t ins, int pos) {
	return ins | (1 << pos);
}


uint32_t multiply(char **tokens) {
	uint32_t ins  = 0;
	if (!strcmp(tokens[0], "mla")) {
		//mla
		ins = setBit(ins, A_BIT);
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
	ins = ins | cond << COND;
	int constField = 9;
	ins = ins | constField << MULTIPLY_CONST;
	return ins;
}