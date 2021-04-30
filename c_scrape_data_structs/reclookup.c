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
    int ifile;
    FILE  *ofile = stdout;
    char *in_file_name = NULL;
    char *out_file_name = NULL;
    int opt;

    while((opt = getopt(argc, argv, "i:o:h")) != -1) {
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

            case 'h':
                printf("%s [-h] [-o file] [-i file]\n", argv[0]);
                exit(EXIT_SUCCESS);

            default:
                fprintf(stderr, " invalid option entered\n");
                exit(EXIT_FAILURE);
            }
    }
    if (in_file_name == NULL) {
        fprintf(stderr, " must include input file with -i\n");
        exit(3);
    }
    if (optind < argc) {
        int i;
        for (i=optind; i<argc; i++) {
            file_struct_t record;
            int rec_num = atoi(argv[i]);                    //get record number from command line
            int offset = sizeof(file_struct_t) * rec_num;   //create offset size in bytes


            lseek(ifile, offset, SEEK_SET);                 //set position in file
            read(ifile, &record, sizeof(file_struct_t));    //read record at lseek position

            //print record
            fprintf(ofile, "id: %s\n first_name: %s\n middle_name: %s\n "
                    "last_name: %s\n street: %s\n city: %s\n "
                    "zip: %s\n country: %s\n email: %s\n phone: %s\n\n",
                    record.id, record.fname, record.mname, record.lname,
                    record.street, record.city, record.zip,
                    record.country_code, record.email, record.phone);
        }
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