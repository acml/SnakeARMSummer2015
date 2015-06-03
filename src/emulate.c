#include <stdlib.h>

#include "pipeline.h"
#include "utils.h"

/*
 * Creates a state, reads in the binary file
 * then performs the fetch-execute cycle
 * contains EXIT codes from the stdlib header
 */
int main(int argc, char **argv) {
    state_t *state = newState();
	
    if (state == NULL) {

        return EXIT_FAILURE;

    }

    if (!readBinary(state, argc, argv)) {
        return EXIT_FAILURE;
    }

    while (!state->isTermainated) {
        execute(state);
        if (!state->isTermainated) {
            decode(state);
            fetch(state);
            incPC(state);
        }
    }
	
    if (!outputState(state)) {
		
        return EXIT_FAILURE;
    }

    delState(state);
    return EXIT_SUCCESS;
}





