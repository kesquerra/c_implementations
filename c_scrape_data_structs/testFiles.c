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
                    char* ls_argv[] = {
                        "ls",
                        0x0
                    };

                    if (dup2(pipe1[STDOUT_FILENO], STDOUT_FILENO) < 0) {
                        perror("ls pipe failed");
                        _exit(2);
                    }
                    close(pipe1[STDIN_FILENO]);
                    close(pipe1[STDOUT_FILENO]);
                    
                    execvp(ls_argv[0], ls_argv);
                    perror("ls exec failed");
                    _exit(3);
                    break;
                }
            default:
                {
                    printf("head started");
                    char* head_argv[] = {
                        "head",
                        "-n",
                        lines,
                        0x0
                    };

                    if (dup2(pipe2[STDIN_FILENO], STDIN_FILENO) < 0) {
                        perror("head pipe failed");
                        exit(11);
                    }
                    close(pipe2[STDIN_FILENO]);
                    close(pipe2[STDOUT_FILENO]);

                    execvp(head_argv[0], head_argv);
                    perror("head exec failed");
                    exit(5);
                }
        }
        

    }

    return EXIT_SUCCESS;
}