#include <stdio.h>
#include <string.h>

#define PROMPT "shell> "
#define MAX_BUFFER_LENGTH 256

int main(int argc, char* argv[]) {
    if (argc > 1) {
        fprintf(stderr, "Don't call me with any arguments.\n");
	return 1;
    }

    printf(PROMPT);

    char buffer[MAX_BUFFER_LENGTH];

    unsigned buflen = 0;
    do {
        buffer[buflen] = getc(stdin);
	buflen += 1;
    } while (buflen < MAX_BUFFER_LENGTH && buffer[buflen-1] != 0x0a);
    buffer[buflen-1] = 0x00;

    if (strcmp(buffer, "exit") != 0) {
        printf("Command other than exit.\n");
    } else {
        printf("Exit called.\n");
    }

    printf("Goodbye.\n");
    return 0;
}
