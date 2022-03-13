#include <stdio.h>
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

    char* args[MAX_BUFFER_LENGTH];
} Command;

Command ParseCommand(char* input) {
    return (Command) { .tag = CommandTag_User, .args = {"ls", NULL} };
        //{ .tag = CommandTag_Absolute, .arglist = ["/bin/ls", "-la", NULL]};
        //{ .tag = CommandTag_Builtin, .arglist = ["proc", "1/status", "NULL"]};
        //{ .tag = CommandTag_Builtin, .arglist = ["exit", "0"]};
}

char* GetCommandName(Command cmd) {
    return cmd.args[0];
}

int main(int argc, char* argv[]) {

    if (argc > 1) {
        fprintf(stderr, "Don't call me with any arguments.\n");
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
                printf("Find command using the absolute path.\n");
                break;
            case CommandTag_User:
                printf("Find command using the current PATH variable.\n");
                break;
            case CommandTag_Relative:
                printf("Find command using a relative path.\n");
                break;
            case CommandTag_Builtin:
                printf("Command is either 'exit' or 'proc'.\n");
                break;
            default:
                fprintf(stderr, "This shouldn't happen.\n");
                return 1;
        }

        printf("You called: %s\n", GetCommandName(cmd));
        break;
    }

    printf("Goodbye.\n");
    return 0;
}
