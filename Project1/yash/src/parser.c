#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <parser.h>

Command *parse_input(const char *input) {
    const int buffer_size = 64;

    Command *command = (Command *) malloc(sizeof(Command));
    if (!command) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    command->argv = malloc(buffer_size * sizeof(char *));
    if (!command->argv) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    command->in_file = NULL;
    command->out_file = NULL;
    command->err_file = NULL;
    command->is_background = 0;
    command->is_piped = 0;
    command->pipe_command = NULL;

    char *input_copy = strdup(input); // copy the input string
    if (!input_copy) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    const size_t len = strlen(input_copy); // remove newline character if present
    if (len > 0 && input_copy[len - 1] == '\n') {
        input_copy[len - 1] = '\0';
    }

    char *pipe_pos = strstr(input_copy, "|");
    if (pipe_pos) {
        *pipe_pos = '\0';

        command->is_piped = 1;
        command->pipe_command = parse_input(pipe_pos + 1); // Recursively parse right side
    }

    char *token = strtok(input_copy, " ");
    int i = 0;
    while (token != NULL) {
        if (strcmp(token, "<") == 0) { // input redirection
            token = strtok(NULL, " ");
            if (!token) {
                fprintf(stderr, "yash: missing input file after '<'\n");
                exit(EXIT_FAILURE);
            }
            command->in_file = strdup(token);
        } else if (strcmp(token, ">") == 0) { // output redirection
            token = strtok(NULL, " ");
            if (!token) {
                fprintf(stderr, "yash: missing output file after '>'\n");
                exit(EXIT_FAILURE);
            }
            command->out_file = strdup(token);
        } else if (strcmp(token, "2>") == 0) { // error redirection
            token = strtok(NULL, " ");
            if (!token) {
                fprintf(stderr, "yash: missing error file after '2>'\n");
                exit(EXIT_FAILURE);
            }
            command->err_file = strdup(token);
        } else if (strcmp(token, "&") == 0) { // background process
            command->is_background = 1;
        } else { // normal command
            command->argv[i] = strdup(token);
            i++;
        }

        token = strtok(NULL, " ");
    }

    command->argv[i] = NULL; // need to null terminate at the end
    free(input_copy);

    return command;
}

void free_command(Command *command) {
    if (command->pipe_command) {
        free_command(command->pipe_command);
    }

    for (int i = 0; command->argv[i] != NULL; i++) {
        free(command->argv[i]);
    }
    free(command->argv);
    free(command->in_file);
    free(command->out_file);
    free(command->err_file);
    free(command);
}

