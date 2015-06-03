#ifndef DEFS_H
#define DEFS_H

#define BYTES_IN_WORD 4
#define BITS_IN_WORD 32
#define REGISTERS_COUNT 17
#define MEMORY_SIZE 65536
#define TOP_BIT 31
#define N_BIT 31
#define Z_BIT 30
#define C_BIT 29
#define V_BIT 28
#define I_BIT 25
#define S_BIT 20
#define A_BIT 21
#define P_BIT 24
#define U_BIT 23
#define L_BIT 20
#define SP_REG 13
#define LR_REG 14
#define PC_REG 15
#define CPSR_REG 16
#define PC_AHEAD_BYTES 8


/*
 * Enum that holds the type of instruction
 */
typedef enum {
    DATA_PROCESSING,
    MULTIPLY,
    SINGLE_DATA_TRANSFER,
    BRANCH,
    TERMINATION
} ins_t;

/*
 * ENum that holds condition types for instruction execution
 */
typedef enum {
    EQ = 0,
    NE = 1,
    GE = 10,
    LT = 11,
    GT = 12,
    LE = 13,
    AL = 14
} cond_t;

/*
 * Enum that holds Data Processing instruction types
 */
typedef enum {
    AND = 0,
    EOR = 1,
    SUB = 2,
    RSB = 3,
    ADD = 4,
    TST = 8,
    TEQ = 9,
    CMP = 10,
    ORR = 12,
    MOV = 13
} opcode_t;

/*
 * Holds the shift types for shifter
 */
typedef enum {
    LSL = 0,
    LSR = 1,
    ASR = 2,
    ROR = 3
} shift_t;

#endif
