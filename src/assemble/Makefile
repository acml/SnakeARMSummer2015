CC      = gcc
CFLAGS  = -Wall -g -D_POSIX_SOURCE -D_BSD_SOURCE -std=c99 -Werror -pedantic

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble

# Build rule for the final executable
assemble: twopasses.o instructions.o binarywriter.o maps.o map.o

# Build rules for the .o files
twopasses.o: twopasses.h instructions.h binarywriter.h maps.h definitions.h

instructions.o: instructions.h binarywriter.h maps.h definitions.h

binarywriter.o: binarywriter.h definitions.h

maps.o: maps.h map.h

map.o: map.h

# Rule to clean generated files
clean:
	rm -f $(wildcard *.o)
	rm -f assemble
