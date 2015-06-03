CC      = gcc
CFLAGS  = -Wall -g -D_POSIX_SOURCE -D_BSD_SOURCE -std=c99 -Werror -pedantic

.SUFFIXES: .c .o

.PHONY: all clean

####main executables#######################

all: assemble emulate

assemble: assemble.o

emulate:  pipeline.o emulate.o utils.o instructions.o 

########################################

pipeline.o: pipeline.h instructions.h

emulate.o: pipeline.h utils.h 

instructions.o: instructions.h utils.h

utils.o: utils.h structs.h definitions.h

##pipeline: instructions.o pipeline.o


##instructions: instructions.o

##utils: utils.o

clean:
	rm -f $(wildcard *.o)
	rm -f assemble
	rm -f emulate
