#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define MAX_LINES 100

int main(int argc, char *argv[]) {
    FILE *ifile = stdin;
    int line_count = 0;
    int max_lines = 100;
    char buf[BUFFER_SIZE] = {0};
    int i=0;
    char* ragged[max_lines];

    //add each line from file into ragged array
    while (line_count < max_lines && fgets(buf, BUFFER_SIZE, ifile) != NULL) {
      ragged[i] = strdup(buf);              //malloc and copy string into ragged array
      line_count++;                         //inc line count
      i++;                                  //inc ragged array index
    }
    
    for (i=0; i<line_count; i++) {
      printf("  %i: %s", i+1, ragged[i]);   //print each item in ragged array
    }

    for (i=0; i<line_count; i++) {
      free(ragged[i]);                      //free memory allocated by strdup()
    } 


    return(EXIT_SUCCESS);
}