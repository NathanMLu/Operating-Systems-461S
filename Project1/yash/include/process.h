#ifndef PROCESS_H
#define PROCESS_H
#include <parser.h>

void execute_command(Command *cmd);
void setup_redirections(Command *cmd);
int open_for_read(const char *filename);
int open_or_create_for_write(const char *filename);

#endif //PROCESS_H
