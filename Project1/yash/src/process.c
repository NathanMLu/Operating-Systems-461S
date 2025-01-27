#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <process.h>
#include <fcntl.h>
#include <job_control.h>
#include <signals.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int open_for_read(const char *filename);
static int open_or_create_for_write(const char *filename);
static void setup_redirections(Command *cmd);

void execute_command(Command *cmd) {
    if (cmd->is_piped && cmd->pipe_command) {
        if (cmd->is_background) {
            fprintf(stderr, "yash: background processes with pipes are not supported\n");
            return;
        }

        int pipe_fds[2];
        if (pipe(pipe_fds) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        const pid_t pid1 = fork();
        if (pid1 == -1) {
            perror("fork (left pipe)");
            exit(EXIT_FAILURE);
        }

        if (pid1 == 0) {
            setpgid(0, 0); // set the child process group id to its pid
            close(pipe_fds[0]);
            if (dup2(pipe_fds[1], STDOUT_FILENO) == -1) { // set stdout to the pipe write end
                perror("dup2");
                exit(EXIT_FAILURE);
            }
            close(pipe_fds[1]);

            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            setup_redirections(cmd);

            if (execvp(cmd->argv[0], cmd->argv) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }

        setpgid(pid1, pid1);

        const pid_t pid2 = fork();
        if (pid2 == -1) {
            perror("fork (right pipe)");
            exit(EXIT_FAILURE);
        }

        if (pid2 == 0) {
            setpgid(0, pid1);
            close(pipe_fds[1]);
            if (dup2(pipe_fds[0], STDIN_FILENO) == -1) { // set stdin to the pipe read end
                perror("dup2");
                exit(EXIT_FAILURE);
            }
            close(pipe_fds[0]);

            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            setup_redirections(cmd);

            if (execvp(cmd->pipe_command->argv[0], cmd->pipe_command->argv) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }

        setpgid(pid2, pid1);

        close(pipe_fds[0]);
        close(pipe_fds[1]);

        tcsetpgrp(STDIN_FILENO, pid1);

        int status1, status2;
        waitpid(pid1, &status1, WUNTRACED);
        waitpid(pid2, &status2, WUNTRACED);

        tcsetpgrp(STDIN_FILENO, getpid());

        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);

        if (WIFSTOPPED(status1) || WIFSTOPPED(status2)) {
            // according to lab doc, we don't need to support ctrl z to put pipe into background
        }

    } else {
        const pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            setpgid(0, 0); // set the child process group id to its pid
            setup_redirections(cmd);

            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            if (execvp(cmd->argv[0], cmd->argv) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            setpgid(pid, pid);
            if (cmd->is_background) {
                add_job(pid, cmd->argv, 1);
            } else {
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);

                tcsetpgrp(STDIN_FILENO, pid);

                int status;
                waitpid(pid, &status, WUNTRACED);

                tcsetpgrp(STDIN_FILENO, getpid());

                signal(SIGINT, SIG_IGN);
                signal(SIGTSTP, SIG_IGN);

                if (WIFSTOPPED(status)) {
                    add_job(pid, cmd->argv, 0);
                    update_job_status(pid, STOPPED);
                }
            }
        }
    }
}

static void setup_redirections(Command *cmd) {
    if (cmd->in_file) {
        const int fd_in = open_for_read(cmd->in_file);
        if (dup2(fd_in, STDIN_FILENO) == -1) { // replace stdin with in_file
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd_in);
    }
    if (cmd->out_file) {
        const int fd_out = open_or_create_for_write(cmd->out_file);
        if (dup2(fd_out, STDOUT_FILENO) == -1) { // replace stdout with out_file
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd_out);
    }
    if (cmd->err_file) {
        const int fd_err = open_or_create_for_write(cmd->err_file);
        if (dup2(fd_err, STDERR_FILENO) == -1) { // replace stderr with err_file
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd_err);
    }
}

static int open_for_read(const char *filename) {
    const int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open (read)");
        exit(EXIT_FAILURE);
    }
    return fd;
}

static int open_or_create_for_write(const char *filename) {
    const int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        perror("open (write)");
        exit(EXIT_FAILURE);
    }
    return fd;
}