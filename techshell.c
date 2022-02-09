/*
   techshell.c: the third and final programming assignment for CSC 222.
   Group members: Brendan Guillory and David Olivier

   A shell. Supports input and output redirection.
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define INPUT_SIZE_LIMIT 256
#define TOKEN_LIMIT INPUT_SIZE_LIMIT/2
#define DELIM " "

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

    // Was the input length 0 (user pressed enter with no command)?
    // If so, return.
    if (!strlen(input))
        return;

    // First, tokenize input

    // We will use strtok() to tokenize the input. Note: This
    // modifies the original string.
    char* pointer = strtok(input, DELIM);

    // The tokens are stored in tokens[]. The first token is, of course,
    // stored in tokens[0].
    char* tokens[TOKEN_LIMIT];
    tokens[0] = pointer;

    // Was the command one of the three reserved ones (cd, pwd, exit)?
    // If so, do the related action.
    if (!strcmp(tokens[0], "cd")) {
        // strtok(NULL, "") will return a char* to the second token,
        // after the cd. This is the folder we want to change to.
        chdir(strtok(NULL, ""));
        return;
    } else if (!strcmp(tokens[0], "pwd")) {
        // Print pwd and return.
        printf("%s\n", getenv("PWD"));
    } else if (!strcmp(tokens[0], "exit")) {
        // Simply exit.
        exit(0);
    }

    // The command was not one of the three reserved keywords. Thus,
    // tokenize the rest of the input and send a child process to execute
    // the command.

    // Store the number of tokens in num_tokens as we go.
    // We have already done one tokenize operation.
    int num_tokens = 1;
    while (pointer != NULL) {
        // Add the next token to tokens[]
        pointer = strtok(NULL, DELIM);
        tokens[num_tokens] = pointer;
        // Increment num_tokens
        num_tokens++;
    }

    // All the tokens are now stored in tokens[].
    // Create a child process to run the command.
    pid_t pid = fork();
    if (pid) {
        // We are the parent (PID != 0).
        int status;
        waitpid(pid, &status, 0);
        return;
    } else {
        // We are the child process.
        // Make the args[] array that we will feed to execvp().
        char* args[num_tokens+1];
        args[0] = tokens[0];
        args[1] = getenv("PWD");
        for (int i = 2; i < num_tokens+1; i++) {
            args[i] = tokens[i-1];
        }

        // Execute
        execvp(args[0], args);
    }
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
    }
}
