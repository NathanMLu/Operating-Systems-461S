#include <stdio.h>
#include <stdlib.h>
#include "job_control.h"
#include "parser.h"
#include "process.h"
#include "signals.h"

int main(void) {
    char *input;

    while (1) {
        printf("# ");
        fflush(stdout);

        size_t len = 0;
        input = NULL; // getline will allocate memory for input
        if (getline(&input, &len, stdin) == -1) {
            printf("Exiting yash.\n");
            free(input);
            break;
        }

        Command *cmd = parse_input(input);
        if (cmd == NULL) {
            perror("parse_input failed");
            exit(EXIT_FAILURE);
        }

        // printf("Command: ");
        // for (int i = 0; cmd->argv[i] != NULL; i++) {
        //     printf("Token[%d]: %s\n", i, cmd->argv[i]);
        // }
        // printf("\n");
        //
        // if (cmd->in_file) {
        //     printf("Input Redirection: %s\n", cmd->in_file);
        // }
        // if (cmd->out_file) {
        //     printf("Output Redirection: %s\n", cmd->out_file);
        // }
        // if (cmd->err_file) {
        //     printf("Error Redirection: %s\n", cmd->err_file);
        // }
        // if (cmd->is_background) {
        //     printf("Background: Yes\n");
        // } else {
        //     printf("Background: No\n");
        // }
        // if (cmd->is_piped) {
        //     printf("Pipe: Yes\n");
        // } else {
        //     printf("Pipe: No\n");
        // }

        if (cmd->argv[0] != NULL) {
            execute_command(cmd);
        } else {
            fprintf(stderr, "No command entered.\n");
        }

        free_command(cmd);
        free(input);
    }

    return 0;
}
