#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "BennySh.h"

// I have this a global so that I don't have to pass it to every
// function where I might want to use it. Yes, I know global variables
// are frowned upon, but there are a couple useful uses for them.
// This is one.
unsigned short isVerbose = 0;

//int main( int argc, char *argv[] )
int 
process_user_input_simple(void)
{
    char str[MAX_STR_LEN];
    char *ret_val;
    char *raw_cmd;
    cmd_list_t *cmd_list = NULL;
    int cmd_count = 0;
    char prompt[30];

    // Set up a cool user prompt.
    sprintf(prompt, PROMPT_STR " %s :-) ", getenv("LOGNAME"));
    for ( ; ; ) {
        fputs(prompt, stdout);
        memset(str, 0, MAX_STR_LEN);
        ret_val = fgets(str, MAX_STR_LEN, stdin);

        if (NULL == ret_val) {
            // end of input, a control-D was pressed.
            // Bust out of the input loop and go home.
            break;
        }

        // STOMP on the trailing newline returned from fgets(). I should be
        // more careful about STOMPing on the last character, just in case
        // it is something other than a newline, but I'm being bold and STOMPing.
        str[strlen(str) - 1] = 0;
        if (strlen(str) == 0) {
            // An empty command line.
            // Just jump back to the promt and fgets().
            // Don't start telling me I'm going to get cooties by
            // using continue.
            continue;
        }

        if (strcmp(str, EXIT_CMD) == 0) {
            // Pickup your toys and go home. I just hope there are not
            // any memory leaks.
            break;
        }
        // Basic commands are pipe delimited.
        // This is really for Stage 2.
        raw_cmd = strtok(str, PIPE_DELIM);

        cmd_list = (cmd_list_t *) calloc(1, sizeof(cmd_list_t));

        // This block should probably be put into its own function.
        cmd_count = 0;
        while (raw_cmd != NULL ) {
            cmd_t *cmd = (cmd_t *) calloc(1, sizeof(cmd_t));

            cmd->raw_cmd = strdup(raw_cmd);
            cmd->list_location = cmd_count++;

            if (cmd_list->head == NULL) {
                // An empty list.
                cmd_list->tail = cmd_list->head = cmd;
            }
            else {
                // Make this the last in the list of cmds
                cmd_list->tail->next = cmd;
                cmd_list->tail = cmd;
            }
            cmd_list->count++;

            // Get the next raw command.
            raw_cmd = strtok(NULL, PIPE_DELIM);
        }
        // Now that I have a linked list of the pipe delimited commands,
        // go through each individual command.
        parse_commands(cmd_list);

        // This is a really good place to call a function to exec the
        // the commands just parsed from the user's command line.
        exec_commands(cmd_list);

        // We (that includes you) need to free up all the stuff we just
        // allocated from the heap. That linked list is linked lists looks
        // like it will be nasty to free up, but just follow the memory.
        free_list(cmd_list);
        cmd_list = NULL;
    }

    return(EXIT_SUCCESS);
}

void 
simple_argv(int argc, char *argv[] )
{
    int opt;

    while ((opt = getopt(argc, argv, "hv")) != -1) {
        switch (opt) {
        case 'h':
            // help
            // Show something helpful
            fprintf(stdout, "You must be out of your Vulcan mind if you think\n"
                    "I'm going to put helpful things in here.\n\n");
            exit(EXIT_SUCCESS);
            break;
        case 'v':
            // verbose option to anything
            // I have this such that I can have -v on the command line multiple
            // time to increase the verbosity of the output.
            isVerbose++;
            if (isVerbose) {
                fprintf(stderr, "verbose: verbose option selected: %d\n"
                        , isVerbose);
            }
            break;
        case '?':
            fprintf(stderr, "*** Unknown option used, ignoring. ***\n");
            break;
        default:
            fprintf(stderr, "*** Oops, something strange happened <%c> ... ignoring ...***\n", opt);
            break;
        }
    }
}

void 
exec_commands( cmd_list_t *cmds ) 
{
    cmd_t *cmd = cmds->head;

    if (1 == cmds->count) {
        if (!cmd->cmd) {
            // if it is an empty command, bail.
            return;
        }
        if (0 == strcmp(cmd->cmd, CD_CMD)) {
            if (0 == cmd->param_count) {
                //change dir to home
                chdir(getenv("HOME"));
            }
            else {
                // try and cd to the target directory. It would be good to check
                // for errors here.
                if (0 == chdir(cmd->param_list->param)) {
                    //happy chdir
                }
                else {
                    // a sad chdir.  :-(
                }
            }
        }
        else if (0 == strcmp(cmd->cmd, PWD_CMD)) {
            char str[MAXPATHLEN];

            // Fetch the current working directory.
            getcwd(str, MAXPATHLEN); 
            printf(" " PWD_CMD ": %s\n", str);
        }
        else if (0 == strcmp(cmd->cmd, ECHO_CMD)) {
            param_t* curr = cmd->param_list;
            while (curr != NULL) {
                printf("%s ", curr->param);
                curr = curr->next;
            }
            printf("\n");
        }
        else {
            //single external command
            //external commands with arguments and stdin/stdout

            pid_t cpid = -1;
            cpid = fork();
            switch(cpid) {
                case -1: {
                    perror("fork failed");
                    exit(3);
                    break;
                }
                case 0: {
                    char** ext_argv = NULL;
                    int i, count = ((cmd->param_count)+1);
                    FILE* ifile = NULL;
                    FILE* ofile = NULL;
                    param_t* param = NULL;
                    param = cmd->param_list;

                    //create
                    ext_argv = calloc(count*2, sizeof(char*));
                    ext_argv[0] = strdup(cmd->cmd);

                    //add parameters to external arguments
                    for (i=1; i<count; i++) {
                        ext_argv[i] = strdup(param->param);
                        param = param->next;
                    }

                    //if there is an input file, open it and set it to stdin
                    if (cmd->input_file_name != NULL) {
                        ifile = fopen(cmd->input_file_name, "r");
                        if (ifile) {
                            if (dup2(fileno(ifile), STDIN_FILENO) < 0) {
                                perror("in pipe failed");
                                _exit(5);
                            }
                        }
                    }

                    //if there is an output file, open it and set it to stdout
                    if (cmd->output_file_name != NULL) {
                        ofile = fopen(cmd->output_file_name, "a");
                        if (ofile) {
                            if (dup2(fileno(ofile), STDOUT_FILENO) < 0) {
                                perror("out pipe failed");
                                _exit(5);
                            }
                        }
                    }
                    //if there is an input file, close it
                    if (ifile) {
                        fclose(ifile);
                    }

                    //if there is an output file, close it
                    if (ofile) {
                        fclose(ofile);
                    }

                    //execute external arguments
                    execvp(ext_argv[0], ext_argv);
                    perror("external command failed");
                    _exit(3);
                    break;
                }
                default: {
                    wait(&cpid);
                    break;
                }
            }
        }
    }
    else {
        int i;
        int status = -1;
        int p_trail;
        pid_t cpid = -1;
        int pipes[2] = {-1, -1};
        int count;
        FILE* ifile;
        FILE* ofile;
        ifile = NULL;
        ofile = NULL;
        
        for (i=0; i<cmds->count; i++) {
            
            if (i != cmds->count-1) {
                pipe(pipes);
            }
            cpid = fork();
            switch(cpid) {
                case -1: {
                    perror("mult command fork failed");
                    exit(3);
                    break;
                }
                case 0: {
                    char** ext_argv = NULL;
                    param_t* param = NULL;
                    param = cmd->param_list;
                    count = ((cmd->param_count)+1);

                    if (i == 0) {
                        if (cmd->input_file_name != NULL) {
                            ifile = fopen(cmd->input_file_name, "r");
                            if (ifile) {
                                if (dup2(fileno(ifile), STDIN_FILENO) < 0) {
                                    perror("in pipe failed");
                                    _exit(5);
                                }
                                fclose(ifile);
                            }
                        }
                    }

                    
                    if (i == cmds->count-1) {
                        if (cmd->output_file_name != NULL) {
                            ofile = fopen(cmd->output_file_name, "a");
                            if (ofile) {
                                if (dup2(fileno(ofile), STDOUT_FILENO) < 0) {
                                    perror("out pipe failed");
                                    _exit(5);
                                }
                                fclose(ofile);
                            }
                        }
                    }

                    if (i != 0) {
                        if (dup2(p_trail, STDIN_FILENO) < 0) {
                            perror("child in pipe failed");
                            _exit(2);
                        }
                    }

                    if (i != cmds->count-1) {
                        if (dup2(pipes[STDOUT_FILENO], STDOUT_FILENO) < 0) {
                            perror("child out pipe failed");
                            _exit(2);
                        }
                        close(pipes[STDIN_FILENO]);
                        close(pipes[STDOUT_FILENO]);
                    }

                    //create
                    ext_argv = calloc(count*2, sizeof(char*));
                    ext_argv[0] = strdup(cmd->cmd);

                    //add parameters to external arguments
                    for (i=1; i<count; i++) {
                        ext_argv[i] = strdup(param->param);
                        param = param->next;
                    }
                    //execute external arguments
                    execvp(ext_argv[0], ext_argv);
                    perror("external command failed");
                    _exit(3);
                    break;
                }
                default: {
                    if (i != 0) {
                        close(p_trail);
                    }
                    if (i != cmds->count-1) {
                        close(pipes[STDOUT_FILENO]);
                        p_trail = pipes[STDIN_FILENO];
                    }
                }
            }
            cmd = cmd->next;
        }
        while ((cpid = wait(&status)) > 0) {
            if (!WIFEXITED(status)) {
                fprintf(stderr, " child process %d failed", cpid);
            }
        }
    }
}

void 
free_list(cmd_list_t *cmd_list)
{
    cmd_t* curr = cmd_list->head;
    while (curr != NULL) {
        cmd_t* tmp = curr->next;
        free_cmd(curr);
        curr = tmp;
    }
    free(cmd_list);
}

void 
print_list(cmd_list_t *cmd_list)
{
    cmd_t *cmd = cmd_list->head;

    while (NULL != cmd) {
        print_cmd(cmd);
        cmd = cmd->next;
    }
}

void 
free_cmd (cmd_t *cmd)
{
    free_param(cmd->param_list);
    free(cmd->raw_cmd);
    free(cmd->cmd);
    free(cmd->input_file_name);
    free(cmd->output_file_name);
    free(cmd);
}

void free_param(param_t *param) {
    if (param != NULL) {
        param_t* curr = param;

        while (curr != NULL) {
            param_t* tmp = curr->next;
            free(curr->param);
            free(curr);
            curr = tmp;
        }
    }

}
// Oooooo, this is nice. Show the fully parsed command line in a nice
// asy to read and digest format.
void 
print_cmd(cmd_t *cmd)
{
    param_t *param = NULL;
    int pcount = 1;

    fprintf(stderr,"raw text: +%s+\n", cmd->raw_cmd);
    fprintf(stderr,"\tbase command: +%s+\n", cmd->cmd);
    fprintf(stderr,"\tparam count: %d\n", cmd->param_count);
    param = cmd->param_list;

    while (NULL != param) {
        fprintf(stderr,"\t\tparam %d: %s\n", pcount, param->param);
        param = param->next;
        pcount++;
    }

    fprintf(stderr,"\tinput source: %s\n"
            , (cmd->input_src == REDIRECT_FILE ? "redirect file" :
               (cmd->input_src == REDIRECT_PIPE ? "redirect pipe" : "redirect none")));
    fprintf(stderr,"\toutput dest:  %s\n"
            , (cmd->output_dest == REDIRECT_FILE ? "redirect file" :
               (cmd->output_dest == REDIRECT_PIPE ? "redirect pipe" : "redirect none")));
    fprintf(stderr,"\tinput file name:  %s\n"
            , (NULL == cmd->input_file_name ? "<na>" : cmd->input_file_name));
    fprintf(stderr,"\toutput file name: %s\n"
            , (NULL == cmd->output_file_name ? "<na>" : cmd->output_file_name));
    fprintf(stderr,"\tlocation in list of commands: %d\n", cmd->list_location);
    fprintf(stderr,"\n");
}

// Remember how I told you that use of alloca() is
// dangerous? You can trust me. I'm a professional.
// And, if you mention this in class, I'll deny it
// ever happened. What happens in stralloca stays in
// stralloca.
#define stralloca(_R,_S) {(_R) = alloca(strlen(_S) + 1); strcpy(_R,_S);}

void 
parse_commands(cmd_list_t *cmd_list)
{
    cmd_t *cmd = cmd_list->head;
    char *arg;
    char *raw;

    while (cmd) {
        // Because I'm going to be calling strtok() on the string, which does
        // alter the string, I want to make a copy of it. That's why I strdup()
        // it.
        // Given that command lines should not be tooooo long, this might
        // be a reasonable place to try out alloca(), to replace the strdup()
        // used below. It would reduce heap fragmentation.
        //raw = strdup(cmd->raw_cmd);

        // Following my comments and trying out alloca() in here. I feel the rush
        // of excitement from the pending doom of alloca(), from a macro even.
        // It's like double exciting.
        stralloca(raw, cmd->raw_cmd);

        arg = strtok(raw, SPACE_DELIM);
        if (NULL == arg) {
            // The way I've done this is like ya'know way UGLY.
            // Please, look away.
            // If the first command from the command line is empty,
            // ignore it and move to the next command.
            // No need free with alloca memory.
            //free(raw);
            cmd = cmd->next;
            // I guess I could put everything below in an else block.
            continue;
        }
        // I put something in here to strip out the single quotes if
        // they are the first/last characters in arg.
        if (arg[0] == '\'') {
            arg++;
        }
        if (arg[strlen(arg) - 1] == '\'') {
            arg[strlen(arg) - 1] = '\0';
        }
        cmd->cmd = strdup(arg);
        // Initialize these to the default values.
        cmd->input_src = REDIRECT_NONE;
        cmd->output_dest = REDIRECT_NONE;

        while ((arg = strtok(NULL, SPACE_DELIM)) != NULL) {
            if (strcmp(arg, REDIR_IN) == 0) {
                // redirect stdin

                //
                // If the input_src is something other than REDIRECT_NONE, then
                // this is an improper command.
                //

                // If this is anything other than the FIRST cmd in the list,
                // then this is an error.

                cmd->input_file_name = strdup(strtok(NULL, SPACE_DELIM));
                cmd->input_src = REDIRECT_FILE;
            }
            else if (strcmp(arg, REDIR_OUT) == 0) {
                // redirect stdout
                       
                //
                // If the output_dest is something other than REDIRECT_NONE, then
                // this is an improper command.
                //

                // If this is anything other than the LAST cmd in the list,
                // then this is an error.

                cmd->output_file_name = strdup(strtok(NULL, SPACE_DELIM));
                cmd->output_dest = REDIRECT_FILE;
            }
            else {
                // add next param
                param_t *param = (param_t *) calloc(1, sizeof(param_t));
                param_t *cparam = cmd->param_list;

                cmd->param_count++;
                // Put something in here to strip out the single quotes if
                // they are the first/last characters in arg.
                if (arg[0] == '\'') {
                    arg++;
                }
                if (arg[strlen(arg) - 1] == '\'') {
                    arg[strlen(arg) - 1] = '\0';
                }
                param->param = strdup(arg);
                if (NULL == cparam) {
                    cmd->param_list = param;
                }
                else {
                    // I should put a tail pointer on this.
                    while (cparam->next != NULL) {
                        cparam = cparam->next;
                    }
                    cparam->next = param;
                }
            }
        }
        // This could overwite some bogus file redirection.
        if (cmd->list_location > 0) {
            cmd->input_src = REDIRECT_PIPE;
        }
        if (cmd->list_location < (cmd_list->count - 1)) {
            cmd->output_dest = REDIRECT_PIPE;
        }

        // No need free with alloca memory.
        //free(raw);
        cmd = cmd->next;
    }

    if (isVerbose > 0) {
        print_list(cmd_list);
    }
}

int 
main( int argc, char *argv[] )
{
    int ret = 0;

    simple_argv(argc, argv);
    ret = process_user_input_simple();

    return(ret);
}
