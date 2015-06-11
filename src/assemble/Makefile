CC      = gcc
CFLAGS  = -Wall -g -D_POSIX_SOURCE -D_BSD_SOURCE -std=c99 -Werror -pedantic

.SUFFIXES: .c .o

.PHONY: all clean

####main executable#######################

all: assemble

assemble: assemble.o map.o maps.o twopasses.o instructions.o binarywriter.o

map.o : map.h

maps.o : maps.h

twopasses.o: twopasses.h

instructions.o: instructions.h

binarywriter.o: binarywriter.h

clean:
	rm -f $(wildcard *.o)
	rm -f assemble
