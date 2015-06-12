#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/*
 * CONSTANTS:
 *
 * Helper constants for memory and address position
 */
#define MEMORY_SIZE 65536
#define MAX_LINE_LENGTH 512
#define MAX_TOKEN_LENGTH 17
#define BITS_IN_BYTE 8
#define BYTES_IN_WORD 4

/*
 * Constants for positions in all instructions
 */
#define COND_POS 28
#define RN_POS 16
#define RD_POS 12
#define RS_POS 8
#define RM_POS 0

/*
 * Constants for representing specific bit positions
 */
#define I_BIT 25
#define S_BIT 20
#define A_BIT 21
#define P_BIT 24
#define U_BIT 23
#define L_BIT 20

/*
 * Constants for shifting bits positions access
 */
#define IMM_ROTATE_POS 8
#define IMM_VALUE_POS 0
#define SHIFT_VALUE_POS 7
#define SHIFT_TYPE_POS 5
#define REG_SHIFT_CONST_POS 4

/*
 * Constant for positions in data processing instructions
 */
#define OPCODE_POS 21

/*
 * Constants for positions in multiply instructions
 */
#define MULTIPLY_RD_POS 16
#define MULTIPLY_RN_POS 12
#define MULTIPLY_CONST_POS 4

/*
 * Constants for positions in single data transfer instructions
 */
#define SINGLE_DATA_TRANSFER_CONST_POS 26
#define MAX_OFFSET_LENGTH 7
#define SINGLE_DATA_TRANSFER_OFFSET_POS 0

/*
 * Constants for positions in branch instructions
 */
#define PC_AHEAD_BYTES 8
#define BRANCH_CONST_POS 25
#define BRANCH_OFFSET_POS 0
#define BRANCH_L_BIT 24

/*
 * Extension constants
 */
#define BX_CONST_POS 4
#define PUSH_POP_CONST_POS 16

#endif
