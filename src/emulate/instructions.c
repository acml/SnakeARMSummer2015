#include <stdio.h>
#include <assert.h>

#include "instructions.h"

int isGpioAddress(uint32_t address);

/*
 * Executes one of data processing instructions. Asserts that none of the
 * registers involved is PC. Alters only CPSR flags in case of TST, CMP, TEQ.
 * In other cases it writes the result to rd.
 */
void dataProcessing(state_t *state) {
    decoded_t *decoded = state->decoded;

    assert(decoded->rd != PC_REG && decoded->rn != PC_REG &&
           decoded->rs != PC_REG && decoded->rm != PC_REG);

    uint32_t operand2;
    int carry = 0;
    if (decoded->isI) {
        operand2 = decoded->immValue;
    } else {
        shift_o output = shiftReg(state);
        operand2 = output.data;
        carry = output.carry;
    }

    //switches on the opcode and then carries out the operation
    uint32_t result = 0;
    switch (decoded->opcode) {
        case AND:
        case TST:
            result = state->registers[decoded->rn] & operand2;
            break;
        case EOR:
        case TEQ:
            result = state->registers[decoded->rn] ^ operand2;
            break;
        case SUB:
        case CMP:
            result = (int32_t) state->registers[decoded->rn]
                    - (int32_t) operand2;
            carry = state->registers[decoded->rn] >= operand2;
            break;
        case RSB:
            result = (int32_t) operand2
                    - (int32_t) state->registers[decoded->rn];
            carry = operand2 >= state->registers[decoded->rn];
            break;
        case ADD:
            result = (int32_t) state->registers[decoded->rn]
                    + (int32_t) operand2;
            carry = state->registers[decoded->rn] > UINT32_MAX - operand2;
            break;
        case ORR:
            result = state->registers[decoded->rn] | operand2;
            break;
        case MOV:
            result = operand2;
            break;
    }

    //switches on the opcode once again
    //determines when to writes the result to rd
    switch (decoded->opcode) {
        case TST:
        case TEQ:
        case CMP:
            break;
        default:
            state->registers[decoded->rd] = result;
            break;
    }

    //sets the flags if needed
    if (decoded->isS) {
        setFlag(state, carry, C_BIT);
        if (result == 0) {
            setFlag(state, 1, Z_BIT);
        } else {
            setFlag(state, 0, Z_BIT);
        }
        setFlag(state, maskBits(result, TOP_BIT, TOP_BIT), N_BIT);
    }
}

/*
 * Executes the multiply instruction.
 * Asserts that none of the registers involved can be PC
 * Also asserts that destination register can't be the same as rm.
 */
void multiply(state_t *state) {
    decoded_t *decoded = state->decoded;

    assert(decoded->rd != PC_REG && decoded->rn != PC_REG &&
           decoded->rs != PC_REG && decoded->rm != PC_REG);
    assert(decoded->rd != decoded->rm);

    uint32_t result = state->registers[decoded->rs]
            * state->registers[decoded->rm];
    if (decoded->isA) {
        result += state->registers[decoded->rn];
    }
    state->registers[decoded->rd] = result;

    if (decoded->isS) {
        if (result == 0) {
            setFlag(state, 1, Z_BIT);
        } else {
            setFlag(state, 0, Z_BIT);
        }
        setFlag(state, maskBits(result, TOP_BIT, TOP_BIT), N_BIT);
    }
}

/*
 * Executes single data transfer instruction. Checks if the instruction to be
 * executed is in the bounds of memory size and Gpio addresses. If not, it
 * outputs the error and keeps working.
 *
 * If the code is loading from GPIO address, the loaded value is equal to that
 * address.
 */
void singleDataTransfer(state_t *state) {
    decoded_t *decoded = state->decoded;

    assert(decoded->rm != PC_REG && decoded->rd != PC_REG);
    assert(decoded->isP || decoded->rm != decoded->rn);

    uint32_t offset;
    if (decoded->isI) {
        offset = shiftReg(state).data;
    } else {
        offset = decoded->immValue;
    }
    if (!decoded->isU) {
        offset = -offset;
    }

    uint32_t address = state->registers[decoded->rn];
    if (decoded->isP) {
        address += offset;
    }

    int isGpio = isGpioAddress(address);
    if (address > MEMORY_SIZE && !isGpio) {
        printf("Error: Out of bounds memory access at address 0x%08x\n",
                address);
        return;
    }

    if (!isGpio) {
        if (decoded->isL) {
            state->registers[decoded->rd] =
                    *((uint32_t *) &state->memory[address]);
        } else {
            *((uint32_t *) &state->memory[address]) =
                    state->registers[decoded->rd];
        }
    } else if (decoded->isL) {
        state->registers[decoded->rd] = address;
    }

    if (!decoded->isP) {
        address += offset;
        if (decoded->rn == PC_REG) {
            address += PC_AHEAD_BYTES;
        }
        state->registers[decoded->rn] = address;
    }
}

/*
 * Checks if the single data transfer is accessing one of GPIO adresses
 * If yes, it outputs the corresponding message
 * Returns 1 if it is a GPIO address and 0 if it is not.
 */
int isGpioAddress(uint32_t address) {
    switch (address) {
        case 0x20200028:
            printf("PIN OFF\n");
            break;
        case 0x2020001C:
            printf("PIN ON\n");
            break;
        case 0x20200000:
            printf("One GPIO pin from 0 to 9 has been accessed\n");
            break;
        case 0x20200004:
            printf("One GPIO pin from 10 to 19 has been accessed\n");
            break;
        case 0x20200008:
            printf("One GPIO pin from 20 to 29 has been accessed\n");
            break;
        default:
            return 0;
    }
    return 1;
}

/*
 * Executes branch instruction. Changes the PC according to the offset
 * and empties the pipeline.
 */
void branch(state_t *state) {
    state->registers[PC_REG] += state->decoded->branchOffset;
    state->isDecoded = 0;
    state->isFetched = 0;
}
