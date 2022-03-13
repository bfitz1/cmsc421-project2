#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PROMPT "shell> "
#define MAX_BUFFER_LENGTH 256

typedef struct Command {
    enum {
        CommandTag_Absolute,
        CommandTag_User,
        CommandTag_Relative,
        CommandTag_Builtin,
    } tag;

    int argc;
    char* argv[MAX_BUFFER_LENGTH];
} Command;

Command ParseCommand(char* input) {
    // Just testing things for now
    
    return
        /*
        (Command) {
            .tag = CommandTag_User,
            .argc = 1,
            .argv = {"ls", NULL},
        };
        */
        /*
        (Command) {
            .tag = CommandTag_Absolute,
            .argc = 2
            .argv = {"/bin/ls", "-la", NULL},
        };
        */
        (Command) {
            .tag = CommandTag_Builtin,
            .argc = 2,
            .argv = {"proc", "1/status", "NULL"},
        };
        /*
        (Command) {
            .tag = CommandTag_Builtin,
            .argc = 1,
            .argv = {"exit"}
        };
        */
        /*
        (Command) {
            .tag = CommandTag_Builtin,
            .argc = 2,
            .argv = {"exit", "9"}
        };
        */
}

int main(int argc, char* argv[]) {

    if (argc > 1) {
        perror("Don't call me with any arguments.");
        return 1;
    }

    char buffer[MAX_BUFFER_LENGTH];

    while (1) {
        printf(PROMPT);

        unsigned buflen = 0;
        do {
            buffer[buflen] = getc(stdin);
            buflen += 1;
        } while (buflen < MAX_BUFFER_LENGTH && buffer[buflen-1] != 0x0a);
        buffer[buflen-1] = 0x00;

        Command cmd = ParseCommand(buffer);
        switch (cmd.tag) {
            case CommandTag_Absolute:
            case CommandTag_User:
            case CommandTag_Relative:
                printf("It is a command!\n");

                int pid = fork();
                if (pid > 0) {
                    wait(NULL);
                } else if (pid == 0) {
                    execvp(cmd.argv[0], cmd.argv);
                    perror("Failed at exec!");
                    exit(1);
                } else {
                    perror("Failed at fork!");
                }
                break;

            case CommandTag_Builtin:
                printf("It is a builtin!\n");
                if (strcmp(cmd.argv[0], "proc") == 0) {
                    printf("It is a proc call!\n");
                    snprintf(buffer, (size_t) MAX_BUFFER_LENGTH, "/%s/%s", cmd.argv[0], cmd.argv[1]);
                    printf("Reading from %s\n", buffer);
                    FILE* proc = fopen("/proc/1/status", "r");
                    int c;
                    while ((c = fgetc(proc)) != EOF) {
                        printf("%c", c);
                    }
                    fclose(proc);
                } else if (strcmp(cmd.argv[0], "exit") == 0) {
                    printf("It is an exit call!\n");
                    if (cmd.argc == 1) {
                        exit(0);
                    } else if (isdigit(cmd.argv[1][0])) {
                       // Remember to free any allocated memory!
                       exit(atoi(cmd.argv[1])); 
                    }
                } else {
                    perror("Failed at builtin!");
                    return 1;
                }
                break;

            default:
                fprintf(stderr, "This shouldn't happen!\n");
                return 1;
        }

        
        printf("You called: %s\n", cmd.argv[0]);
        break;
    }

    printf("Goodbye.\n");
    return 0;
}
