#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <signals.h>

static void handle_sigint(int sig);
static void handle_sigtstp(int sig);

void setup_signal_handlers() {
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);
}

static void handle_sigint(int sig) {
    write(STDOUT_FILENO, "\nCaught SIGINT (Ctrl-C)\n", 24);
}

static void handle_sigtstp(int sig) {
    write(STDOUT_FILENO, "\nCaught SIGTSTP (Ctrl-Z)\n", 25);
}