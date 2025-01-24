#ifndef PARSER_H
#define PARSER_H

char **parse_input(const char *input);
void free_parsed_input(char **tokens);

#endif //PARSER_H