CC = gcc -g -Wall -Wshadow -Wunreachable-code -Wredundant-decls
CC += -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes
CC += -Wdeclaration-after-statement -Wuninitialized -pthread 
DEBUG =

all: primesMT

$(PROG): primesMT.o
		$(CC) -o $@ $^

primesMT.o: primesMT.c
		$(CC) $(DEBUG) -c $<

clean:
		rm -f *.o primesMT *~ \#*