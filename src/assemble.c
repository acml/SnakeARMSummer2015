#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
	if (!strcmp(tokens[0]), "mla") {
		//mla
		ins = setBit(ins, A_BIT);
		int rn =  strtol(token[4] + 1);
		ins = ins | rn << MULTIPLY_RN;
	}
	int rs = strtol(token[3] + 1);
	int rm = strtol(token[2] + 1);
	int rd = strtol(token[1] + 1);
	ins = ins | rs << MULTIPLY_RS;
	ins = ins | rm << MULTIPLY_RM;
	ins = ins | rd << MULTIPLY_RD;
	int cond = 0xe;
	ins = ins | cond << COND;
	int constField = 9;
	ins = ins | constField << MULTIPLY_CONST;
	return ins;
}

int main()
{
   char str[80] = "mov r0, [1, =3] ";
   char  **strArrPtr = tokens(str);
   for(int i = 0 ; i < 10; i++) {
   	if (strArrPtr[i] == NULL) {
   		break;
   	}
   	printf("%s\n", strArrPtr[i]);
   }

   return(0);
}