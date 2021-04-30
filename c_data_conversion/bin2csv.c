#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "file_struct.h"

#define BUFFER_SIZE 100000
#define MAX_LINES 1024
#define TRUE 1
#define FALSE 0

int main(int argc, char *argv[]) {
    int ifile = stdin;
    FILE  *ofile = stdout;
    //char buf[BUFFER_SIZE] = {0};
    char *in_file_name = NULL;
    char *out_file_name = NULL;
    int opt;
    char cma[1];
    char headers[72];
    file_struct_t bin_struct;
    //char delim[] = ",";                                   //delimiters for words in line

    while((opt = getopt(argc, argv, "i:o:vh")) != -1) {
        switch(opt) {
            case 'i':
                in_file_name = optarg;
                ifile = open(in_file_name, O_RDONLY);
                //ifile = fopen(file_name, "r");
                if (in_file_name == NULL) {
                    perror("failed to open input file");
                    fprintf(stderr, " could not open input file: %s\n", in_file_name);
                    exit(EXIT_FAILURE);
                }
                break;

            case 'o':
                out_file_name = optarg;
                ofile = fopen(out_file_name, "w");
                //ofile = open(bin_file_name, O_WRONLY | O_CREAT);
                if (out_file_name == NULL) {
                    perror("failed to open output file");
                    fprintf(stderr, " could not open output file: %s\n", out_file_name);
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
    strcpy(headers,"id,first_name,middle_name,last_name,street,city,zip,country,email,phone\n");
    strcpy(cma, ",");
    fwrite(headers, sizeof(char), sizeof(headers), ofile);

    
    
    
    while (read(ifile, &bin_struct, sizeof(file_struct_t)) > 0) {
        fwrite(&bin_struct.id, sizeof(char) * strlen(bin_struct.id), 1, ofile);
        fwrite(cma, sizeof(cma), 1, ofile);
        fwrite(&bin_struct.fname, sizeof(char) * strlen(bin_struct.fname), 1, ofile);
        fwrite(cma, sizeof(cma), 1, ofile);
        fwrite(&bin_struct.mname, sizeof(char) * strlen(bin_struct.mname), 1, ofile);
        fwrite(cma, sizeof(cma), 1, ofile);
        fwrite(&bin_struct.lname, sizeof(char) * strlen(bin_struct.lname), 1, ofile);
        fwrite(cma, sizeof(cma), 1, ofile);
        fwrite(&bin_struct.street, sizeof(char) * strlen(bin_struct.street), 1, ofile);
        fwrite(cma, sizeof(cma), 1, ofile);
        fwrite(&bin_struct.city, sizeof(char) * strlen(bin_struct.city), 1, ofile);
        fwrite(cma, sizeof(cma), 1, ofile);
        fwrite(&bin_struct.zip, sizeof(char) * strlen(bin_struct.zip), 1, ofile);
        fwrite(cma, sizeof(cma), 1, ofile);
        fwrite(&bin_struct.country_code, sizeof(char) * strlen(bin_struct.country_code), 1, ofile);
        fwrite(cma, sizeof(cma), 1, ofile);
        fwrite(&bin_struct.email, sizeof(char) * strlen(bin_struct.email), 1, ofile);
        fwrite(cma, sizeof(cma), 1, ofile);
        fwrite(&bin_struct.phone, sizeof(char) * strlen(bin_struct.phone), 1, ofile);
        fwrite(cma, sizeof(cma), 1, ofile);
        fwrite("\n", 1, 1, ofile);

    }

    
    //close file if opened
    if (in_file_name != NULL) {
        close(ifile);
    }
    if (out_file_name != NULL) {
        fclose(ofile);
    }

    return(EXIT_SUCCESS);
}
