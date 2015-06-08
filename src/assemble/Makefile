CC      = gcc
CFLAGS  = -Wall -g -D_POSIX_SOURCE -D_BSD_SOURCE -std=c99 -Werror -pedantic

.SUFFIXES: .c .o

.PHONY: all clean

####main executable#######################

all: assemble

assemble: assemble.o map.o assemblyFunctions.o instructions.o binaryWriter.o


map.o : map.h

assemblyFunctions.o: assemblyFunctions.h

instructions.o: instructions.h

binaryWriter.o: binaryWriter.h

clean:
	rm -f $(wildcard *.o)
	rm -f assemble

