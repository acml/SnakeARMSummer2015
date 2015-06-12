#include <string.h>

#include "pipeline.h"
#include "instructions.h"

ins_t insType(uint32_t ins);
int checkCond(state_t *state);

/*
 * Checks if the instruction has been decoded yet
 * If decoded, then checks for termination
 * If still running, then checks the conditions on the current instruction
 * Finally, switches on the instruction to decide what to execute
 */
void execute(state_t *state) {
    if (!state->isDecoded) {
        return;
    }

    decoded_t *decoded = state->decoded;

    if (decoded->ins == TERMINATION) {
        state->isTermainated = 1;
        return;
    }

    if (!checkCond(state)) {
        return;
    }

    switch (decoded->ins) {
        case DATA_PROCESSING:
            dataProcessing(state);
            break;
        case MULTIPLY:
            multiply(state);
            break;
        case SINGLE_DATA_TRANSFER:
            singleDataTransfer(state);
            break;
        case BRANCH:
            branch(state);
            break;
        default:
            break;
    }
}

/*
 * Checks if the data has been fetched
 * Initialises all the values in the decoded_t struct
 * Sets the "isDecoded" field once completed
 */
void decode(state_t *state) {
    //returns if instruction not fetched
    if (!state->isFetched) {
        state->isDecoded = 0;
        return;
    }

    decoded_t *decoded = state->decoded;

    //clears the data currently pointed to before using it
    memset(decoded, 0, sizeof(decoded_t));
    uint32_t instruction = state->fetched;

    //sets the instruction type field
    decoded->ins = insType(instruction);

    //sets the fields which require only individual bits
    decoded->isRegShiftValue = maskBits(instruction, 4, 4);
    decoded->isI = maskBits(instruction, I_BIT, I_BIT);
    decoded->isS = maskBits(instruction, S_BIT, S_BIT);
    decoded->isA = maskBits(instruction, A_BIT, A_BIT);
    decoded->isP = maskBits(instruction, P_BIT, P_BIT);
    decoded->isU = maskBits(instruction, U_BIT, U_BIT);
    decoded->isL = maskBits(instruction, L_BIT, L_BIT);

    //sets the fields which are registers
    if (decoded->ins != BRANCH) {
        decoded->rd = maskBits(instruction, 15, 12);
        decoded->rn = maskBits(instruction, 19, 16);
        if (decoded->ins == MULTIPLY) {
            uint32_t tmp = decoded->rd;
            decoded->rd = decoded->rn;
            decoded->rn = tmp;
            decoded->rs = maskBits(instruction, 11, 8);
            decoded->rm = maskBits(instruction, 3, 0);
        } else if ((decoded->ins == DATA_PROCESSING && !decoded->isI) ||
                   (decoded->ins == SINGLE_DATA_TRANSFER && decoded->isI)) {
            decoded->rm = maskBits(instruction, 3, 0);
            if (decoded->isRegShiftValue) {
                decoded->rs = maskBits(instruction, 11, 8);
            }
        }
    }

    //set the conditions, opcode, type of shift, and shifter values
    decoded->cond = maskBits(instruction, 31, 28);
    decoded->opcode = maskBits(instruction, 24, 21);
    decoded->shift = maskBits(instruction, 6, 5);
    decoded->shiftValue = maskBits(instruction, 11, 7);

    //does the shifting at this stage if needed
    //if no need to shift, just passes the value
    if (decoded->ins == DATA_PROCESSING) {
        uint32_t data = maskBits(instruction, 7, 0);
        uint32_t shiftValue = maskBits(instruction, 11, 8) * 2;
        decoded->immValue = shiftData(data, ROR, shiftValue);
    } else {
        decoded->immValue = maskBits(instruction, 11, 0);
    }

    //calculates the offset for the branch instruction
    uint32_t data = maskBits(instruction, 23, 0);
    decoded->branchOffset = shiftData(shiftData(data, LSL, 8), ASR, 6);

    //informs pipeline that the instruction was decoded
    state->isDecoded = 1;
}

/*
 * Returns the instruction at address stored in PC.
 * Also changes the state of pipeline so it know that instruction is fetched.
 */
void fetch(state_t *state) {
    state->fetched = ((uint32_t *) state->memory)[state->registers[PC_REG]
            / BYTES_IN_WORD];
    state->isFetched = 1;
}

/*
 * Increments the PC by number of bytes in word (moves PC to next address).
 */
void incPC(state_t *state) {
    state->registers[PC_REG] += BYTES_IN_WORD;
}

/*
 * Returns the type of the instruction being decoded.
 */
ins_t insType(uint32_t ins) {
    if (ins == 0) {
        return TERMINATION;
    }
    uint32_t insBits = maskBits(ins, 27, 26);
    if (insBits == 1) {
        return SINGLE_DATA_TRANSFER;
    } else if (insBits == 2) {
        return BRANCH;
    } else if (maskBits(ins, I_BIT, I_BIT) == 0 && maskBits(ins, 7, 4) == 9) {
        return MULTIPLY;
    } else {
        return DATA_PROCESSING;
    }
}

/*
 * Checks if the condition is satisfied before execution of an instruction.
 * Returns 1 if it is, 0 if not.
 */
int checkCond(state_t *state) {
    switch (state->decoded->cond) {
        case AL:
            return 1;
        case EQ:
            return getFLag(state, Z_BIT);
        case NE:
            return !getFLag(state, Z_BIT);
        case GE:
            return getFLag(state, N_BIT) == getFLag(state, V_BIT);
        case LT:
            return getFLag(state, N_BIT) != getFLag(state, V_BIT);
        case GT:
            return !getFLag(state, Z_BIT)
                    && getFLag(state, N_BIT) == getFLag(state, V_BIT);
        case LE:
            return getFLag(state, Z_BIT)
                    || getFLag(state, N_BIT) != getFLag(state, V_BIT);
    }
    return 0;
}
