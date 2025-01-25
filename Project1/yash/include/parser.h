#ifndef PARSER_H
#define PARSER_H

typedef struct {
    char **argv;
    char *in_file; // <
    char *out_file; // >
    char *err_file; // 2>
    int is_background; // 1 if it is a background process
    int is_piped; // 1 if it is a piped process
} Command;

Command *parse_input(const char *input);
void free_command(Command *command);

#endif //PARSER_H
