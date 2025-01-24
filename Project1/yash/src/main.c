#include <stdio.h>
#include <stdlib.h>
#include "job_control.h"
#include "parser.h"
#include "process.h"
#include "signals.h"

int main() {
    char *input;

    while (1) {
        printf("# ");
        fflush(stdout);

        size_t len = 0;
        input = NULL; // we don't need to allocate because getline will allocate
        if (getline(&input, &len, stdin) == -1) {
            printf("Exiting yash.\n");
            free(input);
            break;
        }

        char **tokens = parse_input(input);
        if (tokens == NULL) {
            perror("parse_input failed");
            exit(EXIT_FAILURE); // Exit if tokenization fails
        }

        for (int i = 0; tokens[i] != NULL; i++) {
            printf("Token[%d]: %s\n", i, tokens[i]);
        }

        if (tokens[0] != NULL) {
            execute_command(tokens);
        } else {
            fprintf(stderr, "No command entered.\n");
        }

        free_parsed_input(tokens);
        free(input);
    }

    return 0;
}
