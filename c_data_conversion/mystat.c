#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>



typedef struct mystat {
    struct stat* file;
    char* name;
    ino_t inode;
    dev_t device;
    char* mode;
    nlink_t links;
    uid_t user_id;
    gid_t group_id;
    blksize_t block_size;
    blkcnt_t block_alloc;
    off_t file_size;
    char* tla;
    char* tlm;
    char* tlsc;
    char* file_type;
    int octal;

    struct passwd* user;
    struct group* group;
} ms_t;

void format_time(time_t, char*);
void fill_mode(ms_t*, mode_t);
ms_t fill_mystat(char*);
void free_mystat(ms_t*);
void print_mystat(ms_t);

void format_time(time_t time, char* ftime) {
    struct tm* info;
    info = localtime(&time);
    strftime(ftime, 80, "%Y-%m-%d %H:%M:%S %z (%Z) %a", info);
}

void fill_mode(ms_t *ms, mode_t fmode) {
    char tmp_mode[10];
    int i;
    
    if (S_ISREG(fmode)) {
        strcpy(ms->file_type, "regular file");

    } else if (S_ISDIR(fmode)) {
        strcpy(ms->file_type, "directory");
        tmp_mode[0] = 'd';

    } else if (S_ISCHR(fmode)) {
        strcpy(ms->file_type, "character device");
        ms->mode[0] = 'c';
        
    } else if (S_ISBLK(fmode)) {
        strcpy(ms->file_type, "block device");
        ms->mode[0] = 'b';
        
    } else if (S_ISFIFO(fmode)) {
        strcpy(ms->file_type, "FIFO/pipe");
        ms->mode[0] = 'p';
        
    } else if (S_ISSOCK(fmode)) {
        strcpy(ms->file_type, "socket");
        ms->mode[0] = 's';
        
    } else if (S_ISLNK(fmode)) {
        if (readlink(ms->name, ms->file_type, 80) <= 0) {
            strcpy(ms->file_type, "symbolic link with dangling destination");
        } else {
            char *tmp = strdup(ms->file_type);
            strcpy(ms->file_type, "symbolic link -> ");
            strcat(ms->file_type, tmp);
            free(tmp);
        }
        ms->mode[0] = 'l';
        
    } else {
        strcpy(ms->file_type, "does not exit");
    }

    for (i=0; i<10; i++) {
        tmp_mode[i] = '-';
    }
    
    if (fmode & S_IRUSR) {
        tmp_mode[1] = 'r';
        ms->octal += 400;
    }
    if (fmode & S_IWUSR) {
        tmp_mode[2] = 'w';
        ms->octal += 200;
    }
    if (fmode & S_IXUSR) {
        tmp_mode[3] = 'x';
        ms->octal += 100;
    }
    if (fmode & S_IRGRP) {
        tmp_mode[4] = 'r';
        ms->octal += 40;
    }
    if (fmode & S_IWGRP) {
        tmp_mode[5] = 'w';
        ms->octal += 20;
    }
    if (fmode & S_IXGRP) {
        tmp_mode[6] = 'x';
        ms->octal += 10;
    }
    if (fmode & S_IROTH) {
        tmp_mode[7] = 'r';
        ms->octal += 4;
    }
    if (fmode & S_IWOTH) {
        tmp_mode[8] = 'w';
        ms->octal += 2;
    }
    if (fmode & S_IXOTH) {
        tmp_mode[9] = 'x';
        ms->octal += 1;
    }
    printf("%s", tmp_mode);
    ms->mode = tmp_mode;
}

ms_t fill_mystat(char* file_name) {
    ms_t mystat;
    struct stat file;
    lstat(file_name, &file);

    mystat.name = strdup(file_name); //need to free
    mystat.file_type = malloc(sizeof(char)*80); //need to free
    mystat.inode = file.st_ino;
    mystat.device = file.st_dev;
    mystat.mode = malloc(sizeof(char)*10); //need to free
    mystat.links = file.st_nlink;
    mystat.user_id = file.st_uid;
    mystat.group_id = file.st_gid;
    mystat.block_size = file.st_blksize;
    mystat.block_alloc = file.st_blocks;
    mystat.file_size = file.st_size;

    //time management
    mystat.tla = malloc(sizeof(char)*80);   //need to free
    mystat.tlm = malloc(sizeof(char)*80);   //need to free
    mystat.tlsc = malloc(sizeof(char)*80);  //need to free
    format_time(file.st_atime, mystat.tla);
    format_time(file.st_mtime, mystat.tlm);
    format_time(file.st_ctime, mystat.tlsc);

    mystat.user = getpwuid(file.st_uid);    //owner name
    mystat.group = getgrgid(file.st_gid);   //group name

    mystat.octal = 0; //zero out the octal;

    fill_mode(&mystat, file.st_mode); //set mode bits and file type

    //free file type

    return mystat;
}

void free_mystat(ms_t *ms) {
    free(ms->name);
    free(ms->file_type);
    ms->mode = NULL;
    free(ms->mode);
    free(ms->tla);
    free(ms->tlm);
    free(ms->tlsc);
}

void print_mystat(ms_t ms) {
    printf("File: %s\n", ms.name);
    printf("\tFile Type: \t\t\t\t%s\n", ms.file_type);
    printf("\tDevice ID Number:\t\t\t%lu\n", ms.device);
    printf("\tI-node number:\t\t\t\t%lu\n", ms.inode);
    printf("\tMode:\t\t\t\t\t%s\t(%i in octal)\n", ms.mode, ms.octal);
    printf("\tLink Count:\t\t\t\t%lu\n", ms.links);
    printf("\tOwner Id:\t\t\t\t%s\t(UID = %u)\n", ms.user->pw_name, ms.user_id);
    printf("\tGroup Id:\t\t\t\t%s\t\t(GID = %u)\n", ms.group->gr_name, ms.group_id);
    printf("\tPreferred I/O Block Size:\t\t%lu\n", ms.block_size);
    printf("\tFile size:\t\t\t\t%lu\n", ms.file_size);
    printf("\tBlocks allocated\t\t\t%lu\n", ms.block_alloc);
    printf("\tLast file access:\t\t\t%s\n",ms.tla);
    printf("\tLast file modification:\t\t\t%s\n",ms.tlm);
    printf("\tLast status change:\t\t\t%s\n",ms.tlsc);
}

int main(int argc, char *argv[]) {
  FILE *ifile = stdin;
  char *file_name = NULL;
  int i;
  
    for (i=1; i<argc; i++) {
      file_name = argv[i];
      ifile = fopen(file_name, "r");
      if (ifile == NULL) {
        perror("failed to open file");
        fprintf(stderr, " could not open file: %s\n", file_name);
      } else {
        ms_t ms = fill_mystat(file_name);
        print_mystat(ms);
        free_mystat(&ms);

        fclose(ifile);
      }
    }

  return(EXIT_SUCCESS);
}
