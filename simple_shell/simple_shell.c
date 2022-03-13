#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "utils.h"

#define PROMPT "$ "
#define BUFFER_LENGTH 4096

typedef struct argdata_t {
    char* argstring;
    unsigned int arglength;
    char** arglist;
    unsigned int argcount;
} argdata_t;

typedef struct command_t {
    enum {
        kind_process,
        kind_procfs,
        kind_exit,
        kind_invalid,
    } tag;

    union {
        char** list;
        char* path;
        int code;
        char* input;
    };
} command_t;

argdata_t* argdata_make(char* raw, unsigned int len) {
    argdata_t* arg = malloc(sizeof(argdata_t*));

    if (arg == NULL) {
        perror("Error: Couldn't allocate argdata");
        exit(1);
    }

    arg->argstring = strdup(raw);
    arg->arglength = len;
    arg->arglist = NULL; // Figure this out
    arg->argcount = 0;

    return arg;
}

void argdata_free(argdata_t* arg) {
    free(arg->argstring);
    for (unsigned int i = 0; i < arg->argcount; i += 1) {
        free(arg->arglist[i]);
    }
    free(arg);
}

command_t* command_make(argdata_t* arg) {
    command_t* cmd = malloc(sizeof(command_t*));

    // It's just exit for now
    cmd->tag = kind_exit;
    cmd->code = 0;

    return cmd;
}

void command_free(command_t* cmd) {
    switch (cmd->tag) {
        case kind_process:
            break;
        case kind_procfs:
            break;
        case kind_exit:
            break;
        case kind_invalid:
            break;
        default:
            perror("Error: Unreachable at command_free");
            break;
    }

    free(cmd);
}

int main(int argc, char* argv[]) {

    if (argc > 1) {
        perror("Error: Don't call me with any arguments.\n");
        return 1;
    }

    char buffer[BUFFER_LENGTH];
    // Test data
    // command_t cmd = (command_t) { .tag = kind_exit, .code = 7 };
    // command_t cmd = (command_t) { .tag = kind_procfs, .path = "/proc/1/status" };
    // command_t cmd = (command_t) { .tag = kind_invalid, .input = "woops" };
    char** arglist = malloc(sizeof(char*) * 3);
    arglist[0] = "/bin/ls";
    arglist[1] = "-la";
    arglist[2] = NULL;
    command_t cmd = (command_t) { .tag = kind_process, .list = arglist };

    while (1) {
        printf(PROMPT);
        fgets(buffer, BUFFER_LENGTH, stdin);

        if (cmd.tag == kind_process) {
            pid_t pid = fork();
            if (pid > 0) {
                wait(NULL);
            } else if (pid == 0) {
                execvp(cmd.list[0], cmd.list);
            } else {
                perror("Error: Couldn't fork the process");
                return 1;
            }    
        } else if (cmd.tag == kind_procfs) {
            FILE* fd = fopen(cmd.path, "r");
            if (fd == NULL) {
                fprintf(stderr, "Error: Couldn't open %s\n", cmd.path);
                return 1;
            }
            int byte;
            while ((byte = fgetc(fd)) != EOF) {
                printf("%c", byte);
            }
            fclose(fd);
        } else if (cmd.tag == kind_exit) {
            exit(cmd.code);
        } else if (cmd.tag == kind_invalid) {
            // Just move on
            printf("Debugging: %s\n", cmd.input);
        } else {
            fprintf(stderr, "Error: Couldn't match on command");
            return 1;
        }
        break;
    }

    //free(arglist);
    printf("Exiting. Goodbye!\n");
    return 0;
}
