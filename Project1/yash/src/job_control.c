#include <job_control.h>
#include <signal.h>
#include <signals.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>

static Job job_table[MAX_JOBS];
static int current_job_id = 0; // we will use this to assign job ids

void init_job_table(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        job_table[i].job_id = 0;
    }
}

int add_job(pid_t pgid, char **argv, int is_background) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].job_id == 0) {
            current_job_id++;

            job_table[i].job_id = current_job_id;
            job_table[i].pgid = pgid;

            // allocate memory for argv
            int argc = 0;
            while (argv[argc] != NULL) argc++;
            job_table[i].argv = malloc((argc + 1) * sizeof(char *));
            if (!job_table[i].argv) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            // copy the argv array
            for (int j = 0; argv[j] != NULL; j++) {
                job_table[i].argv[j] = strdup(argv[j]);
                if (!job_table[i].argv[j]) {
                    perror("strdup");
                    exit(EXIT_FAILURE);
                }
            }
            job_table[i].argv[argc] = NULL; // null-terminate the array

            job_table[i].status = RUNNING;
            job_table[i].is_background = is_background;

            if (is_background) {
                printf("[%d] %-16s", current_job_id, "Running");
                for (int j = 0; argv[j] != NULL; j++) {
                    printf("%s ", argv[j]);
                }
                printf("&\n");
            }

            return current_job_id;
        }
    }

    fprintf(stderr, "yash: job table is full\n");
    return -1;
}

void update_job_status(pid_t pgid, JobStatus status) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].pgid == pgid) {
            job_table[i].status = status;
            return;
        }
    }
}

void remove_job(pid_t pgid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].pgid == pgid) {
            if (job_table[i].argv) {
                for (int j = 0; job_table[i].argv[j] != NULL; j++) {
                    if (job_table[i].argv[j]) {
                        free(job_table[i].argv[j]);
                        job_table[i].argv[j] = NULL;
                    }
                }
                free(job_table[i].argv);
                job_table[i].argv = NULL;
            }
            job_table[i].job_id = 0;
            return;
        }
    }
}


void check_job_statuses(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].job_id != 0 && job_table[i].status == RUNNING) {
            int status;
            pid_t result = waitpid(job_table[i].pgid, &status, WNOHANG);
            if (result == -1) {
                perror("waitpid");
            } else if (result > 0) {
                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    if (job_table[i].is_background) {
                        printf("\n[%d] %-16s", job_table[i].job_id, "Done");
                        for (int j = 0; job_table[i].argv[j] != NULL; j++) {
                            printf("%s ", job_table[i].argv[j]);
                        }
                        printf("&\n");
                        printf("# ");
                        fflush(stdout);
                    }


                    update_job_status(job_table[i].pgid, DONE);
                }
            }
        }

        if (job_table[i].job_id != 0 && job_table[i].status == DONE) {
            remove_job(job_table[i].pgid);
        }
    }
}


void list_jobs(void) {
    int most_recent = 0;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].job_id > most_recent) {
            most_recent = job_table[i].job_id;
        }
    }

    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].job_id != 0) {
            char job_symbol = '-';
            if (job_table[i].job_id == most_recent) {
                job_symbol = '+';
            }

            printf("[%d]%c %-15s", job_table[i].job_id, job_symbol,
                   job_table[i].status == RUNNING ? "Running" : job_table[i].status == STOPPED ? "Stopped" : "Done");

            for (int j = 0; job_table[i].argv[j] != NULL; j++) {
                printf("%s ", job_table[i].argv[j]);
            }

            if (job_table[i].is_background && job_table[i].status != DONE) {
                printf("&");
            }
            printf("\n");
        }
    }
}


Job *get_recent_job(int is_background) {
    for (int i = MAX_JOBS - 1; i >= 0; i--) {
        if (job_table[i].job_id != 0 && (job_table[i].is_background == is_background || job_table[i].status == STOPPED)) {
            return &job_table[i];
        }
    }
    return NULL;
}

int foreground_job(void) {
    Job *job = get_recent_job(1);
    if (job) {
        update_job_status(job->pgid, RUNNING);
        job->is_background = 0;
        for (int j = 0; job->argv[j] != NULL; j++) {
            printf("%s ", job->argv[j]);
        }
        printf("\n");

        kill(-job->pgid, SIGCONT);

        int status;
        waitpid(job->pgid, &status, WUNTRACED);
        if (WIFSTOPPED(status)) {
            update_job_status(job->pgid, STOPPED);
        } else {
            remove_job(job->pgid);
        }

        return 1;
    }
    return 0;
}

int background_job(void) {
    Job *job = get_recent_job(0);
    if (job) {
        update_job_status(job->pgid, RUNNING);
        job->is_background = 1;
        printf("[%d] %-16s", job->job_id, "Running");
        for (int j = 0; job->argv[j] != NULL; j++) {
            printf("%s ", job->argv[j]);
        }
        printf("&\n");

        kill(-job->pgid, SIGCONT);

        return 1;
    }
    return 0;
}
