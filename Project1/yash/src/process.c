#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <process.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

static int open_for_read(const char *filename);
static int open_or_create_for_write(const char *filename);
static void setup_redirections(Command *cmd);

void execute_command(Command *cmd) {
    if (cmd->is_piped && cmd->pipe_command) {
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
            close(pipe_fds[0]);
            if (dup2(pipe_fds[1], STDOUT_FILENO) == -1) { // set stdout to the pipe write end
                perror("dup2");
                exit(EXIT_FAILURE);
            }
            close(pipe_fds[1]);

            setup_redirections(cmd);

            if (execvp(cmd->argv[0], cmd->argv) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }

        const pid_t pid2 = fork();
        if (pid2 == -1) {
            perror("fork (right pipe)");
            exit(EXIT_FAILURE);
        }

        if (pid2 == 0) {
            close(pipe_fds[1]);
            if (dup2(pipe_fds[0], STDIN_FILENO) == -1) { // set stdin to the pipe read end
                perror("dup2");
                exit(EXIT_FAILURE);
            }
            close(pipe_fds[0]);

            setup_redirections(cmd);

            if (execvp(cmd->pipe_command->argv[0], cmd->pipe_command->argv) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }

        close(pipe_fds[0]);
        close(pipe_fds[1]);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    } else {
        const pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            setup_redirections(cmd);

            if (execvp(cmd->argv[0], cmd->argv) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            // the parent process
            waitpid(pid, NULL, 0);
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