#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>

void print_indent(int depth);
char* cstr_skip_whitespace(char** s, char* end, int expect_more);
void skip_whitespace(FILE* in_file); 

#endif //HELPERS_H
