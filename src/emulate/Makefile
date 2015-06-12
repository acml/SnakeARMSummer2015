CC      = gcc
CFLAGS  = -Wall -g -D_POSIX_SOURCE -D_BSD_SOURCE -std=c99 -Werror -pedantic

.SUFFIXES: .c .o

.PHONY: all clean

all: emulate

# Build rule for the final executable
emulate: pipeline.o instructions.o inputoutput.o utils.o

# Build rules for the .o files
emulate.o: pipeline.h inputoutput.h utils.h

pipeline.o: pipeline.h instructions.h utils.h

instructions.o: instructions.h utils.h

inputoutput.o: inputoutput.h utils.h

utils.o: utils.h definitions.h

# Rule to clean generated files
clean:
	rm -f $(wildcard *.o)
	rm -f emulate
