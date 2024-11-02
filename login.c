#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned int djb2(const char *str) {
    unsigned int hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

void to_hex(unsigned int value, char *output) {
    sprintf(output, "%08x", value);
}

void disable_echo(struct termios *old) {
    struct termios new;
    tcgetattr(STDIN_FILENO, old);
    new = *old;
    new.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
}

void enable_echo(struct termios *old) {
    tcsetattr(STDIN_FILENO, TCSANOW, old);
}

#define PASSWORD_HASH "7c9d79a9"

int main() {
    char input_password[33];
    char hash_output[9];
    struct termios old_term;

    printf("Android     \n\nLogin: root\nPassword: ");
    disable_echo(&old_term);
    fgets(input_password, sizeof(input_password), stdin);
    input_password[strcspn(input_password, "\n")] = 0;
    enable_echo(&old_term);
    printf("\n\n");

    unsigned int hash_value = djb2(input_password);
    to_hex(hash_value, hash_output);

    if (strcmp(hash_output, PASSWORD_HASH) == 0) {
        execl("/system/bin/sh", "sh", NULL);
        printf("Accessing error.\n\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Access denied.\n\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}