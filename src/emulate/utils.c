#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "utils.h"

int shiftCarry(uint32_t data, shift_t shift, uint32_t shiftValue);

/*
 * Allocate memory for a new state_t
 * Initialises all the values in the struct
 * Returns a pointer to the state_t
 */
state_t *newState(void) {
    state_t *state = malloc(sizeof(state_t));
    if (state == NULL) {
        return NULL;
    }

    state->decoded = malloc(sizeof(decoded_t));
    if (state->decoded == NULL) {
        free(state);
        return NULL;
    }

    state->registers = malloc(REGISTERS_COUNT * sizeof(uint32_t));
    if (state->registers == NULL) {
        free(state->decoded);
        free(state);
        return NULL;
    }
    memset(state->registers, 0, REGISTERS_COUNT * sizeof(uint32_t));

    state->memory = malloc(MEMORY_SIZE * sizeof(uint8_t));
    if (state->memory == NULL) {
        free(state->decoded);
        free(state->registers);
        free(state);
        return NULL;
    }
    memset(state->memory, 0, MEMORY_SIZE * sizeof(uint8_t));

    state->fetched = 0;
    state->isDecoded = 0;
    state->isFetched = 0;
    state->isTermainated = 0;

    return state;
}

/*
 * Checks for null pointers
 * Frees all the pointers inside the state
 * Finally frees state itself
 */
void delState(state_t *state) {
    if (state != NULL) {
        if (state->decoded != NULL) {
            free(state->decoded);
        }

        if (state->registers != NULL) {
            free(state->registers);
        }

        if (state->memory != NULL) {
            free(state->memory);
        }

        free(state);
    }
}

/*
 * Returns the value in data between and upper and lower bit (both included).
 */
inline uint32_t maskBits(uint32_t data, int upper, int lower) {
    assert(upper >= lower && upper <= 31 && lower >= 0);
    data <<= TOP_BIT - upper;
    data >>= TOP_BIT - (upper - lower);
    return data;
}

/*
 * Returns the given flag from CPSR_REG
 */
uint32_t getFLag(state_t *state, int flag) {
    return maskBits(state->registers[CPSR_REG], flag, flag);
}

/*
 * Sets or clears (according to val) given flag in CPSR_REG.
 */
void setFlag(state_t *state, int val, int flag) {
    assert(val == 0 || val == 1);
    state->registers[CPSR_REG] ^= (-val ^ state->registers[CPSR_REG])
            & (1 << flag);
}

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
 * For left shift, it is BITS_IN_WORD - shiftValue and for right
 * shifts it is shiftValue - 1.
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
