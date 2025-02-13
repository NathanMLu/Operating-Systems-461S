#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "job_control.h"
#include "parser.h"
#include "process.h"
#include "signals.h"

int main(void) {
    char *input;

    init_job_table();
    setup_signal_handlers();

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

        if (cmd->argv[0] != NULL) {
            if (strcmp(cmd->argv[0], "jobs") == 0) {
                check_job_statuses();
                list_jobs();
            } else if (strcmp(cmd->argv[0], "fg") == 0) {
                if (!foreground_job()) {
                    printf("yash: no jobs to foreground\n");
                }
            } else if (strcmp(cmd->argv[0], "bg") == 0) {
                if (!background_job()) {
                    printf("yash: no stopped jobs to background\n");
                }
            } else {
                execute_command(cmd);
            }
        } else {
            fprintf(stderr, "No command entered.\n");
        }

        free_command(cmd);
        free(input);
    }

    return 0;
}
