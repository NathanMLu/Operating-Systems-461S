#include <job_control.h>
#include <signal.h>
#include <unistd.h>
#include <signals.h>

static void handle_sigchld(int sig);

void setup_signal_handlers(void) {
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, handle_sigchld);
}

static void handle_sigchld(int sig) {
    check_job_statuses();
}





