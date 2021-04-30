CC = gcc -g -Wall -Wshadow -Wunreachable-code -Wredundant-decls
CC += -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes
CC += -Wdeclaration-after-statement
DEBUG =

all: BennySh

$(PROG): BennySh.o
		$(CC) -o $@ $^

BennySh.o: BennySh.c
		$(CC) $(DEBUG) -c $<

clean:
		rm -f *.o BennySh *~ \#*