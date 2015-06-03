#include "instructions.h"

/*
 * It asserts that the shiftValue is smaller than the size of the word.
 * Returns the data shifted by shiftValue using a shift specified. 
 */ 
uint32_t shiftData(uint32_t data, shift_t shift, uint32_t shiftValue) {
    assert(shiftValue < BITS_IN_WORD);
    if (shiftValue == 0) {
        return data;
    }
    switch (shift) {
        case LSL:
            data <<= shiftValue;
            break;
        case LSR:
            data >>= shiftValue;
            break;
        case ASR:
            data = (int32_t) data >> shiftValue;
            break;
        case ROR:
            data = (data >> shiftValue) | (data << (BITS_IN_WORD - shiftValue));
            break;
    }
    return data;
}

/*
 * Asserts that shiftValue is not longer than size of the word.
 * Returns the last bit discarded/rotated by the corresponding shift 
 * For left shift, it is BITS_IN_WORD - shiftvalue and for right 
 * shifts it is shiftvalue - 1.
 */
int shiftCarry(uint32_t data, shift_t shift, uint32_t shiftValue) {
    assert(shiftValue < BITS_IN_WORD);
    if (shiftValue == 0) {
        return 0;
    }
    int carryBit = 0;
    switch (shift) {
        case LSL:
            carryBit = BITS_IN_WORD - shiftValue;
            break;
        case LSR:
        case ASR:
        case ROR:
            carryBit = shiftValue - 1;
            break;
    }
    return maskBits(data, carryBit, carryBit);
}

/*
 * Executes shift register function necessary for computing operand2. 
 * Returns the shift output struct containing shifted data and the carry bit.
 */
shift_o shiftReg(state_t *state) {
    decoded_t *decoded = state->decoded;

    uint32_t shiftValue = decoded->shiftValue;
    if (decoded->isRegShiftValue) {
        shiftValue = maskBits(state->registers[decoded->rs], 7, 0);
    }
    uint32_t data = state->registers[decoded->rm];

    shift_o output;
    output.data = shiftData(data, decoded->shift, shiftValue);
    output.carry = shiftCarry(data, decoded->shift, shiftValue);
    return output;
}


/*
 * Executes one of data processing instructions. Asserts that none of the 
 * registers involved is PC. Alters only CPSR flags in case of TST, CMP, TEQ.
 * In other cases it writes the resul to rd.
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
    //determines when to set value
    switch (decoded->opcode) {
        case TST:
        case TEQ:
        case CMP:
            break;
        default:
            state->registers[decoded->rd] = result;
            break;
    }

    //sets the flags if told to do
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
 * Checks if the single data transfer is accessing one of GPIO adresses
 * If yes, it outputs the corresponding messagge
 * Returns 1 if it is a GPIO address and 0 if it is not.
 */
static int checkGpioAddresses(uint32_t address) {
	if(address == 0x20200028) {
	    printf("PIN OFF\n");
	    return 1;
    }
    if(address == 0x2020001C) {
        printf("PIN ON\n");
        return 1;
    }
    if(address == 0x20200000) {

        printf("One GPIO pin from 0 to 9 has been accessed\n");
        return 1;
    }
    if (address == 0x20200004) {
        printf("One GPIO pin from 10 to 19 has been accessed\n");
        return 1;
    }
    if (address == 0x20200008) {
        printf("One GPIO pin from 20 to 29 has been accessed\n");
        return 1;
    }
    return 0;
}

/*
 * Executes single data transfer instruction. Checks if the instruction to be
 * executed is in the bounds of memory size and gpio adresses. If not, it
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

    if (address > MEMORY_SIZE) {
		if (checkGpioAddresses(address)) {
			if(decoded->isL) {
				state->registers[decoded->rd] = address;
			}
		} else {
            printf("Error: Out of bounds memory access at address 0x%08x\n",
                address);
        }
        return;
    }

    if (decoded->isL) {
        state->registers[decoded->rd] = *((uint32_t *) &state->memory[address]);
    } else {
        *((uint32_t *) &state->memory[address]) = state->registers[decoded->rd];
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
 * Executes branch instruction. Changes the PC according to the offset
 * and empties the pipeline.
 */
void branch(state_t *state) {
    state->registers[PC_REG] += state->decoded->branchOffset;
    state->isDecoded = 0;
    state->isFetched = 0;
}
