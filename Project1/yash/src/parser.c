#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <parser.h>

char **parse_input(const char *input) {
    const int buffer_size = 64;

    char **tokens = (char **) malloc(buffer_size * sizeof(char *));
    if (!tokens) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    char *input_copy = strdup(input); // copy the input since original is const
    if (!input_copy) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    const size_t len = strlen(input_copy); // remove newline character if present
    if (len > 0 && input_copy[len - 1] == '\n') {
        input_copy[len - 1] = '\0';
    }

    char *token = strtok(input_copy, " "); // get the first token
    int i = 0;

    while (token != NULL) {
        tokens[i] = strdup(token);
        if (!tokens[i]) break;

        token = strtok(NULL, " ");
        i++;
    }
    tokens[i] = NULL;

    free(input_copy); // free the copy, we only needed it for tokenization
    return tokens;
}

void free_parsed_input(char **tokens) {
    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
}
