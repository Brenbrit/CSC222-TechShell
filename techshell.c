/*
   techshell.c: the third and final programming assignment for CSC 222.
   Group members: Brendan Guillory and David Olivier

   A shell. Supports input and output redirection.
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_SIZE_LIMIT 256

char* getUserInput() {
    // input is the string we will store the user input in.
    // If we were to write "char* input;" here, we would meet a
    // segfault when removing the newline character a few lines down.
    char* input = (char*) malloc(INPUT_SIZE_LIMIT);

    // Get user input with fgets
    fgets(input, INPUT_SIZE_LIMIT, stdin);

    // The user input will always have a newline thanks to fgets.
    // Remove this newline (replace it with \0).
    input[strlen(input) - 1] = '\0';

    return input;
}

void processCommand(char* input) {

    printf("Input received: \"%s\"\n", input);
    // First, tokenize input
}

// Main function
// Calls other functions and runs until "exit" is entered
int main() {
    // Main loop
    while (1) {
        // Echo pwd to user, followed by "$ "
        printf("%s$ ", getenv("PWD"));

        // Get user input
        char* input = getUserInput();

        // Process command
        processCommand(input);
        exit(0);
    }
}
