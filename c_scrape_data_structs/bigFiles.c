#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    char* lines = "10";

    {
        int opt;
        while ((opt = getopt(argc, argv, "n:h")) != -1) {
            switch (opt) {
                case 'n':
                    lines = optarg;
                    printf("%s", lines);
                    break;
                case 'h':
                    printf("%s [-h] [-n # of files]\n", argv[0]);
                    exit(EXIT_SUCCESS);
                    break;
                default:
                    break;
            }
        }
    }
    
    {
        int pipe1[2] = {-1, -1};
        int pipe2[2] = {-1, -1};
        int pipe3[3] = {-1, -1};
        pid_t cpid = -1;
        char* ls_argv[] = {
            "ls",
            0x0
        };
        char* xargs_argv[] = {
            "xargs",
            "du",
            "-s",
            0x0
        };
        char* sort_argv[] = {
            "sort",
            "-nr",
            0x0
        };
        char* head_argv[] = {
            "head",
            "-n",
            lines,
            0x0
        };

        pipe(pipe1);
        pipe(pipe2);
        pipe(pipe3);
        cpid = fork();
        switch (cpid) {
            case -1:
                perror("ls fork failed");
                exit(6);
                break;
            case 0:
                {
                    printf("ls started");
                    //child process
                    
                    if (dup2(pipe1[STDOUT_FILENO], STDOUT_FILENO) < 0) {
                        perror("ls pipe failed");
                        _exit(2);
                    }
                    close(pipe1[STDIN_FILENO]);
                    close(pipe1[STDOUT_FILENO]);
                    close(pipe2[STDIN_FILENO]);
                    close(pipe2[STDOUT_FILENO]);
                    close(pipe3[STDIN_FILENO]);
                    close(pipe3[STDOUT_FILENO]);
                    
                    execvp(ls_argv[0], ls_argv);
                    perror("ls exec failed");
                    _exit(3);
                    break;
                }
            default:
                {
                    //parent process
                    break;
                }
        }
        cpid = fork();
        switch(cpid) {
            case -1: {
                perror("xargs fork failed");
                exit(7);
                break;
            }
            case 0: {
                printf("xargs started");
                

                if (dup2(pipe1[STDIN_FILENO], STDIN_FILENO) < 0) {
                    perror("xargs in pipe failed");
                    _exit(4);
                }
                
                if (dup2(pipe2[STDOUT_FILENO], STDOUT_FILENO) < 0) {
                    perror("xargs out pipe failed");
                    _exit(5);
                }

                close(pipe1[STDIN_FILENO]);
                close(pipe1[STDOUT_FILENO]);
                close(pipe2[STDIN_FILENO]);
                close(pipe2[STDOUT_FILENO]);
                close(pipe3[STDIN_FILENO]);
                close(pipe3[STDOUT_FILENO]);

                execvp(xargs_argv[0], xargs_argv);
                perror("xargs exec failed");
                _exit(3);
                break;
            }
            default: {
                //parent process
                break;
            }
        }
        
        cpid = fork();
        switch(cpid) {
            case -1: {
                perror("sort fork failed");
                exit(7);
                break;
            }
            case 0: {
                printf("fork started");

                if (dup2(pipe2[STDIN_FILENO], STDIN_FILENO) < 0) {
                    perror("sort in pipe failed");
                    _exit(8);
                }

                if (dup2(pipe3[STDOUT_FILENO], STDOUT_FILENO) < 0) {
                    perror("sort out pipe failed");
                    _exit(9);
                }

                close(pipe1[STDIN_FILENO]);
                close(pipe1[STDOUT_FILENO]);
                close(pipe2[STDIN_FILENO]);
                close(pipe2[STDOUT_FILENO]);
                close(pipe3[STDIN_FILENO]);
                close(pipe3[STDOUT_FILENO]);

                execvp(sort_argv[0], sort_argv);
                perror("sort exec failed");
                _exit(3);
                break;
            }
            default: {
                //parent process
                printf("head started");

                if (dup2(pipe3[STDIN_FILENO], STDIN_FILENO) < 0) {
                    perror("head pipe failed");
                    exit(11);
                }
                close(pipe1[STDIN_FILENO]);
                close(pipe1[STDOUT_FILENO]);
                close(pipe2[STDIN_FILENO]);
                close(pipe2[STDOUT_FILENO]);
                close(pipe3[STDIN_FILENO]);
                close(pipe3[STDOUT_FILENO]);

                execvp(head_argv[0], head_argv);
                perror("head exec failed");
                exit(5);
                break;
            }
        }
        

    }

    return EXIT_SUCCESS;
}