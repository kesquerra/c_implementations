#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "file_struct.h"

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
    int ofile = stdout;
    char buf[BUFFER_SIZE] = {0};
    char *file_name = NULL;
    char *bin_file_name = NULL;
    int opt;
    char delim[] = ",";                                   //delimiters for words in line

    while((opt = getopt(argc, argv, "i:o:vh")) != -1) {
        switch(opt) {
            case 'i':
                file_name = optarg;
                ifile = fopen(file_name, "r");
                if (file_name == NULL) {
                    perror("failed to open input file");
                    fprintf(stderr, " could not open input file: %s\n", file_name);
                    exit(EXIT_FAILURE);
                }
                break;

            case 'o':
                bin_file_name = optarg;
                ofile = open(bin_file_name, O_WRONLY | O_CREAT);
                if (bin_file_name == NULL) {
                    perror("failed to open output file");
                    fprintf(stderr, " could not open output file: %s\n", bin_file_name);
                }
                break;

            case 'v':
                fprintf(stderr, "Diagnostics set\n");
                break;

            case 'h':
                printf("%s [-h] [-v] [-o file] [-i file]\n", argv[0]);
                exit(EXIT_SUCCESS);

            default:
                fprintf(stderr, " invalid option entered\n");
                exit(EXIT_FAILURE);
            }
    }
    fgets(buf, BUFFER_SIZE, ifile);

    while (fgets(buf, BUFFER_SIZE, ifile) != NULL) {
        file_struct_t bin_struct;
        strcpy(bin_struct.id, strtok(buf, delim));
        strcpy(bin_struct.fname, strtok(NULL, delim));
        strcpy(bin_struct.mname, strtok(NULL, delim));
        strcpy(bin_struct.lname, strtok(NULL, delim));
        strcpy(bin_struct.street, strtok(NULL, delim));
        strcpy(bin_struct.city, strtok(NULL, delim));
        strcpy(bin_struct.zip, strtok(NULL, delim));
        strcpy(bin_struct.country_code, strtok(NULL, delim));
        strcpy(bin_struct.email, strtok(NULL, delim));
        strcpy(bin_struct.phone, strtok(NULL, delim));

        write(ofile, &bin_struct, sizeof(file_struct_t)); 
    }

    
    //close file if opened
    if (file_name != NULL) {
        fclose(ifile);
    }
    if (bin_file_name != NULL) {
        close(ofile);
    }

    return(EXIT_SUCCESS);
}
