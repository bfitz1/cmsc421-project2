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

// ArgData is mostly metadata about the current input being process.
// Enough structure to answer some questions and get working
typedef struct ArgData {
    char* argstring;
    unsigned int arglength;
    char** arglist;
    unsigned int argcount;
} ArgData;

// Command is derived from ArgData. It's more structured and has
// everything necessary in order to execute a process or builtin. 
typedef struct Command {
    enum {
        CommandTag_Process,
        CommandTag_ProcFS,
        CommandTag_Exit,
        CommandTag_Invalid,
    } tag;

    union {
        char** list;
        char* path;
        int code;
    };
} Command;

ArgData* create_ArgData(char raw[]) {
    ArgData* arg = malloc(sizeof(ArgData));

    if (!arg) {
        perror("Error: Couldn't allocate ArgData");
        exit(1);
    }

    char* copy = strdup(raw);
    if (!copy) {
        perror("Error: Couldn't duplicate string");
        exit(1);
    }
    arg->argstring = copy;
    arg->arglength = strlen(copy);

    unsigned int cap = MIN_LENGTH;
    char** list = malloc(sizeof(char*) * cap);
    unsigned int count = 0;

    char* start = strdup(raw);
    char* next = start;
    if (!next) {
        perror("Error: Couldn't duplicate string");
        exit(1);
    }

    int pos;
    do {
        pos = first_unquoted_space(next);
        if (pos > -1) {
            // Mark the "end" of the current string to unescape
            next[pos] = '\0';
            list[count] = unescape(next, stderr);

            // Since there's input remaining, this should be safe 
            next += pos + 1;
        } else {
            list[count] = unescape(next, stderr);
        }
        count += 1;

        // Realloc check happens at the end to ensure our list is never full
        // I.E. a cheesy way to have arglist[count] not be out-of-bounds
        if (count == cap) {
            cap *= 2;
            list = (char**) realloc(list, sizeof(char*) * cap);
            if (!list) {
                perror("Error: Couldn't allocate ArgData.arglist");
                exit(1);
            }
        }
    } while (pos > -1);
    free(start);

    list[count] = NULL;
    arg->arglist = list;
    arg->argcount = count;

    return arg;
}

void destroy_ArgData(ArgData* arg) {
    if (!arg) {
        return;
    }

    if (arg->argstring) {
        free(arg->argstring);
        arg->argstring = NULL;
    }

    if (arg->arglist) {
        for (unsigned int i = 0; i < arg->argcount; i += 1) {
            if (arg->arglist[i]) {
                free(arg->arglist[i]);
                arg->arglist[i] = NULL;
            }
        }
    }
    free(arg->arglist);
    arg->arglist = NULL;

    free(arg);
    arg = NULL;
}

Command* create_Command(ArgData* arg) {
    Command* cmd = malloc(sizeof(Command));

    if (strcmp(arg->arglist[0], "exit") == 0) {
        switch (arg->argcount) {
            case 1:
                cmd->tag = CommandTag_Exit;
                cmd->code = 0;
                break;
            case 2:
                // atoi() returns a zero (!) on a failed parse
                // This probably isn't a rigorous enough check but oh well
                if (isdigit(arg->arglist[1][0])) {
                    cmd->tag = CommandTag_Exit;
                    cmd->code = atoi(arg->arglist[1]);
                } else {
                    cmd->tag = CommandTag_Invalid;
                }
                break;
            default:
                cmd->tag = CommandTag_Invalid;
                break;
        }
    } else if (strcmp(arg->arglist[0], "proc") == 0) {
        switch (arg->argcount) {
            case 2:
                cmd->tag = CommandTag_ProcFS;
                cmd->path = malloc(sizeof(char) * (arg->arglength + 8));
                snprintf(cmd->path, (arg->arglength + 8), "/proc/%s", arg->arglist[1]);
                break;
            default:
                cmd->tag = CommandTag_Invalid;
                break;
        }
    } else {
        // Just throw any other input at an exec call and hope for the best
        cmd->tag = CommandTag_Process;
        cmd->list = arg->arglist;

        // Trying not to share pointers in too many different places
        arg->arglist = NULL;
    }

    return cmd;
}

void destroy_Command(Command* cmd) {
    if (!cmd) {
        return;
    }

    switch (cmd->tag) {
        case CommandTag_Process:
            if (cmd->list) {
                for (unsigned int i = 0; cmd->list[i] != NULL; i += 1) {
                    free(cmd->list[i]);
                }
                free(cmd->list);
                cmd->list = NULL;
            }
            break;
        case CommandTag_ProcFS:
            if (cmd->path) {
                free(cmd->path);
                cmd->path = NULL;
            }
            break;
        case CommandTag_Exit:
        case CommandTag_Invalid:
            break;
        default:
            perror("Error: Unreachable at destroy_Command");
            break;
    }

    free(cmd);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        perror("Error: Don't call me with any arguments.");
        return 1;
    }

    char buffer[MIN_LENGTH];

    while (1) {
        printf(PROMPT);
        fgets(buffer, MIN_LENGTH, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        ArgData* arg = create_ArgData(buffer);
        Command* cmd = create_Command(arg);

        if (cmd->tag == CommandTag_Process) {
            pid_t pid = fork();
            if (pid > 0) {
                wait(NULL);
            } else if (pid == 0) {
                execvp(cmd->list[0], cmd->list);
            } else {
                perror("Error: Couldn't fork the process");
                return 1;
            }    
        } else if (cmd->tag == CommandTag_ProcFS) {
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
        } else if (cmd->tag == CommandTag_Exit) {
            int code = cmd->code;

            // Clean up before exiting
            destroy_ArgData(arg);
            destroy_Command(cmd);

            exit(code);
        } else if (cmd->tag == CommandTag_Invalid) {
            // Silently consume invalid commands
        } else {
            perror("Error: Couldn't match on Command");
            return 1;
        }

        // I'm stumped on these two
        destroy_ArgData(arg);
        destroy_Command(cmd);
    }

    return 0;
}
