#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

//gcc -g -Wall -o front3 front3.c
// ./front3 < front3.c
// ./front3 -f front3.c
// ./front3 -f front3.c -n 3
// ./front3 -n 3 -f front3.c
// ./front3 -n 2 < front3.c

#define BUFFER_SIZE 1024
#define MAX_LINES 1024
#define TRUE 1
#define FALSE 0

int main(int argc, char *argv[]) {
    FILE *ifile = stdin;
    int line_count = 0, word_count = 0, char_count = 0;
    char* word;
    int max_lines = MAX_LINES;
    char buf[BUFFER_SIZE] = {0};
    char *file_name = NULL;
    int opt;
    char* line_ptr;
    char delim[] = " \n";                                   //delimiters for words in line
    int option[4] = {FALSE, FALSE, FALSE, FALSE};           //boolean options for opt arguments
    

    //get arguments from command line
    while((opt = getopt(argc, argv, "hcwlf:")) != -1) {
        switch(opt) {
            case 'f':
                file_name = optarg;
                ifile = fopen(file_name, "r");
                if (ifile == NULL) {
                    perror("failed to open file");
                    fprintf(stderr, " could not open file: %s\n", file_name);
                    exit(EXIT_FAILURE);
                }
                option[3] = TRUE;
                break;

            case 'l':
                option[0] = TRUE;
                break;

            case 'w':
                option[1] = TRUE;
                break;

            case 'c':
                option[2] = TRUE;
                break;
      
            case 'h':
                printf("%s [-h] [-c] [-w] [-l] [-f file]\n", argv[0]);
                exit(EXIT_SUCCESS);

            default:
                fprintf(stderr, " invalid option entered\n");
                exit(EXIT_FAILURE);
            }
    }

    //get counts for lines, words and chars
    while (line_count < max_lines && fgets(buf, BUFFER_SIZE, ifile) != NULL) {
        line_count++;                                           //inc line count
        line_ptr = buf;                                         //pointer to buffer
        char_count += strlen(line_ptr);                         //add char length of line
        while ((word = strtok(line_ptr, delim)) != NULL) {      //tokenize each word in line
            word_count++;                                       //inc word count
            line_ptr = NULL;                                    //set pointer to null
        }
    }

    //if no optarg besides -f, then default to -lwc
    if (!option[0] && !option[1] && !option[2]) {
        option[0] = TRUE;
        option[1] = TRUE;
        option[2] = TRUE;
    }

    //if -l optarg
    if (option[0]) {
        printf("%i ", line_count);
    }
    //if -w optarg
    if (option[1]) {
        printf("%i ", word_count);
    }

    //if -c optarg
    if (option[2]) {
        printf("%i ", char_count);
    }
    
    //if -f optarg
    if (option[3]) {
        printf("%s", file_name);
    }

    printf("\n");

    //close file if opened
    if (file_name != NULL) {
        fclose(ifile);
    }

    return(EXIT_SUCCESS);
}
