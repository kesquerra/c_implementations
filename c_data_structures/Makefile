CC = gcc -g -Wall -Wshadow -Wunreachable-code -Wredundant-decls
CC += -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes
CC += -Wdeclaration-after-statement
PROG = lab4
DEBUG =

all: $(PROG)

$(PROG): mywc.o csv2bin.o bin2csv.o
		$(CC) -o $@ $^

mywc.o: mywc.c
		$(CC) $(DEBUG) -c $<

csv2bin.o: csv2bin.c
		$(CC) $(DEBUG) -c $<

bin2csv.o: bin2csv.c
		$(CC) $(DEBUG) -c $<

clean:
		rm -f *.o mystat *~ \#*
		rm -f *.o bin2csv *~ \#*
		rm -f *.o csv2bin *~ \#*