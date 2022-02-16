/*
  _______        _      _____ _          _ _
 |__   __|      | |    / ____| |        | | |
    | | ___  ___| |__ | (___ | |__   ___| | |
    | |/ _ \/ __| '_ \ \___ \| '_ \ / _ \ | |
    | |  __/ (__| | | |____) | | | |  __/ | |
    |_|\___|\___|_| |_|_____/|_| |_|\___|_|_|

   techshell.c: the third and final programming assignment for CSC 222.
   Group members: Brendan Guillory and David Olivier

   A shell. Supports input, output, and error redirection.
   */

// For reading errors
#include <errno.h>
// For using sockets between child and parent processes
#include <fcntl.h>
// For standard io
#include <stdio.h>
// General library with lots of useful functions
#include <stdlib.h>
// For string computations
#include <string.h>
// For one function: wait().
#include <sys/wait.h>
// Has functions for reading CWD and reading/writing with pipes
#include <unistd.h>

#define INPUT_SIZE_LIMIT 256
#define TOKEN_LIMIT INPUT_SIZE_LIMIT/2
#define CWD_SIZE_LIMIT 256
#define DELIM " "

// Global cwd char array
char cwd[CWD_SIZE_LIMIT];

// Helper function to update the global cwd variable.
// Called at the beginning and after every cd.
void updateCWD() {
    getcwd(cwd, CWD_SIZE_LIMIT);
}

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

// Checks if we are running a built-in command.
// If we are, do it and return 0.
// Else, return 1
int builtInCommands(char* command) {

    if (!strcmp(command, "cd")) {
        // strtok(NULL, "") will return a char* to the second token,
        // after the cd. This is the folder we want to change to.
        char* new_dir = strtok(NULL, "");

        // Try to change directories with chdir
        int cd_status = chdir(new_dir);

        // Did chdir() actually do anything? Let's check!
        if (cd_status == 0) {
            // Success!
            // Update the global cwd variable
            updateCWD();
            return 0;
        } else {
            // chdir() failed. Tell the user why.
            fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno));
            return 0;
        }
    } else if (!strcmp(command, "pwd")) {
        // Print pwd and return.
        printf("%s\n", cwd);
        return 0;
    } else if (!strcmp(command, "exit")) {
        // Simply exit.
        exit(0);
    }

    return 1;
}


void processCommand(char* input) {

    // Was the input length 0 (user pressed enter with no command)?
    // If so, return.
    if (!strlen(input))
        return;

    // Reset input, output and error redirections
    // redirections[0] = input file
    // redirections[1] = output file
    // redirections[2] = error file
    char* redirections[3] = {NULL, NULL, NULL};

    // First, tokenize input

    // We will use strtok() to tokenize the input. Note: This
    // modifies the original string.
    char* pointer = strtok(input, DELIM);

    // Did the user type a space (or a bunch of spaces)?
    // If so, just exit.
    if (pointer == NULL)
        return;

    // The tokens are stored in tokens[]. The first token is, of course,
    // stored in tokens[0].
    char* tokens[TOKEN_LIMIT];
    tokens[0] = pointer;

    // Was the command one of the three reserved ones (cd, pwd, exit)?
    // If so, do the related action.
    if (builtInCommands(pointer) == 0)
        return;

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

    // Set up input and output files (if necessary)
    // Scan through each token
    for (int i = 0; tokens[i] != NULL; i++) {

        // If we found >, <, or 2>
        if (!strcmp(tokens[i], "<")
                || !strcmp(tokens[i], ">")
                || !strcmp(tokens[i], "2>")) {

            // Is the token following the >, <,  or 2> even there?
            if (tokens[i+1] != NULL) {
                // Set the relevant variable
                if (!strcmp(tokens[i], "<")) {
                    redirections[0] = tokens[i+1];
                } else if (!strcmp(tokens[i], ">")) {
                    redirections[1] = tokens[i+1];
                } else {
                    redirections[2] = tokens[i+1];
                }

                // Remove i and i+1 from tokens[]
                for (int j = i+2; j < num_tokens; j++)
                    tokens[j-2] = tokens[j];
                // Decrease num_tokens by 2
                num_tokens -= 2;
                // Decrement i to prevent skipping tokens
                i--;
            }
        }
    }

    // Set up error handling.
    int pipefds[2];
    int count, error;
    // pipe(pipefds) is used to communicate from the child to the parent.
    if (pipe(pipefds)) {
        fprintf(stderr, "Error making error-handling pipe\n");
    }
    // fcntl is used to open the socket between the child and parent.
    if (fcntl(pipefds[1], F_SETFD, fcntl(pipefds[1], F_GETFD) | FD_CLOEXEC)) {
        fprintf(stderr, "Error using fcntl\n");
    }

    // Create a child process to run the command.
    pid_t pid = fork();
    if (pid != 0) {
        // We are the parent (PID != 0).

        // Did the child's execvp() do something strange?
        // Get return status of execvp() from the pipe.
        close(pipefds[1]);
        while ((count = read(pipefds[0], &error, sizeof(int))) == -1)
            if (errno != EAGAIN && errno != EINTR) break;
        if (count) {
            fprintf(stderr, "Error %d (%s)\n", error, strerror(error));
        }

        wait(NULL);

        return;
    } else {
        // We are the child process

        // Close pipe[0]
        close(pipefds[0]);

        // Set input, output, and error files
        FILE* redirection_files[3] = {
            fopen(redirections[0], "r"),
            fopen(redirections[1], "w"),
            fopen(redirections[2], "w")
        };

        // Redirect input, output and stderr if necessary
        for (int i = 0; i < 3; i++)
            if (redirections[i] != NULL)
                dup2(fileno(redirection_files[i]), i);

        // Execute execvp
        execvp(tokens[0], tokens);

        // Close input and output files
        for (int i  = 0; i < 3; i++)
            if (redirections[i] != NULL)
                fclose(redirection_files[i]);

        // Write the error code of execvp to the pipe[1].
        // This will not happen if execvp runs correctly.
        // &errno describes the error. sizeof(int) is the amount
        // of data to write.
        write(pipefds[1], &errno, sizeof(int));

        // We are done! Exit the child process.
        exit(0);
    }
}


// Main function
// Calls other functions and runs until "exit" is entered
int main() {

    // Update CWD to the initial value
    updateCWD();

    // Main loop
    while (1) {
        // Echo pwd to user, followed by "$ "
        printf("%s$ ", cwd);

        // Get user input
        char* input = getUserInput();

        // Process command
        processCommand(input);
    }
}
