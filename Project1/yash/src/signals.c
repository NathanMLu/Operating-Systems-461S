#include <job_control.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <signals.h>

static void handle_sigint(int sig);
static void handle_sigtstp(int sig);
static void handle_sigchld(int sig);

void setup_signal_handlers() {
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);
    signal(SIGCHLD, handle_sigchld);
}

static void handle_sigint(int sig) {
    write(STDOUT_FILENO, "\nyash: SIGINT received\n", 23);
}


static void handle_sigtstp(int sig) {
    write(STDOUT_FILENO, "\nyash: SIGTSTP received\n ", 24);
}

static void handle_sigchld(int sig) {
    check_job_statuses();
}
