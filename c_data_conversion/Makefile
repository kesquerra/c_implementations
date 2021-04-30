CC = gcc -g -Wall -Wshadow -Wunreachable-code -Wredundant-decls
CC += -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes
CC += -Wdeclaration-after-statement
DEBUG =

all: mystat csv2bin bin2csv

$(PROG): mystat.o csv2bin.o bin2csv.o
		$(CC) -o $@ $^

mystat.o: mystat.c
		$(CC) $(DEBUG) -c $<

csv2bin.o: csv2bin.c
		$(CC) $(DEBUG) -c $<

bin2csv.o: bin2csv.c
		$(CC) $(DEBUG) -c $<

clean:
		rm -f *.o mystat *~ \#*
		rm -f *.o bin2csv *~ \#*
		rm -f *.o csv2bin *~ \#*