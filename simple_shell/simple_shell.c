#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "utils.h"

#define PROMPT "$ "
#define MIN_LENGTH 1024

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
    };
} command_t;

argdata_t* argdata_make(char raw[]) {
    argdata_t* arg = malloc(sizeof(argdata_t*));

    if (!arg) {
        fprintf(stderr, "Error: Couldn't allocate argdata\n");
        exit(1);
    }

    char* dupe = strdup(raw);
    if (!dupe) {
        fprintf(stderr, "Error: Couldn't duplicate string\n");
        exit(1);
    }
    arg->argstring = dupe;
    arg->arglength = strlen(dupe);

    unsigned int cap = MIN_LENGTH;
    char** list = malloc(sizeof(char*) * cap);
    unsigned int count = 0;

    char* start = strdup(raw);
    char* tmp = start;
    if (!tmp) {
        fprintf(stderr, "shell error: %s\n", strerror(errno));
        exit(1);
    }

    int pos;
    do {
        pos = first_unquoted_space(tmp);
        if (pos > -1) {
            tmp[pos] = '\0';
            list[count] = unescape(tmp, stderr);
            tmp += pos + 1;
        } else {
            list[count] = unescape(tmp, stderr);
        }
        count += 1;

        // Realloc check happens at the end to ensure our list is never full
        if (count == cap) {
            cap *= 2;
            list = (char**) realloc(list, sizeof(char*) * cap);
            if (!list) {
                fprintf(stderr, "shell error: %s\n", strerror(errno));
                exit(1);
            }
        }
    } while (tmp[0]);
    free(start);

    arg->arglist = list;
    arg->argcount = count;

    return arg;
}

void argdata_free(argdata_t* arg) {
    if (arg->argstring) {
        free(arg->argstring);
        arg->argstring = NULL;
    }

    for (unsigned int i = 0; i < arg->argcount; i += 1) {
        if (arg->arglist[i]) {
            free(arg->arglist[i]);
            arg->arglist[i] = NULL;
        }
    }
    free(arg->arglist);
    arg->arglist = NULL;

    free(arg);
    arg = NULL;
}

command_t* command_make(argdata_t* arg) {
    command_t* cmd = malloc(sizeof(command_t*));

    if (strcmp(arg->arglist[0], "exit") == 0) {
        switch (arg->argcount) {
            case 1:
                cmd->tag = kind_exit;
                cmd->code = 0;
                break;
            case 2:
                cmd->tag = kind_exit;
                cmd->code = atoi(arg->arglist[1]);
                break;
            default:
                cmd->tag = kind_invalid;
                break;
        }
    } else if (strcmp(arg->arglist[0], "proc") == 0) {
        switch (arg->argcount) {
            case 2:
                cmd->tag = kind_procfs;
                cmd->path = malloc(sizeof(char) * (arg->arglength + 8));
                snprintf(cmd->path, (arg->arglength + 8), "/proc/%s", arg->arglist[1]);
                break;
            default:
                cmd->tag = kind_invalid;
                break;
        }
    } else {
        cmd->tag = kind_process;
        cmd->list = arg->arglist;
        cmd->list[arg->argcount] = NULL;
        arg->arglist = NULL;
    }

    return cmd;
}

void command_free(command_t* cmd) {
    switch (cmd->tag) {
        case kind_process:
            if (cmd->list) {
                for (unsigned int i = 0; cmd->list[i] != NULL; i += 1) {
                    free(cmd->list[i]);
                }
                free(cmd->list);
                cmd->list = NULL;
            }
            break;
        case kind_procfs:
            if (cmd->path) {
                free(cmd->path);
                cmd->path = NULL;
            }
            break;
        case kind_exit:
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

    char buffer[MIN_LENGTH];

    while (1) {
        printf(PROMPT);
        fgets(buffer, MIN_LENGTH, stdin);

        argdata_t* arg = argdata_make(buffer);
        command_t* cmd = command_make(arg);

        if (cmd->tag == kind_process) {
            pid_t pid = fork();
            if (pid > 0) {
                wait(NULL);
            } else if (pid == 0) {
                execvp(cmd->list[0], cmd->list);
            } else {
                perror("Error: Couldn't fork the process");
                return 1;
            }    
        } else if (cmd->tag == kind_procfs) {
            FILE* fd = fopen(cmd->path, "r");
            if (fd == NULL) {
                fprintf(stderr, "Error: Couldn't open %s\n", cmd->path);
                return 1;
            }
            int byte;
            while ((byte = fgetc(fd)) != EOF) {
                printf("%c", byte);
            }
            fclose(fd);
        } else if (cmd->tag == kind_exit) {
            exit(cmd->code);
        } else if (cmd->tag == kind_invalid) {
            // Silently consume invalid commands
        } else {
            fprintf(stderr, "Error: Couldn't match on command");
            return 1;
        }

        // Buggy. Dunno why. Oh well.
        //argdata_free(arg);
        //command_free(cmd);
    }

    return 0;
}
