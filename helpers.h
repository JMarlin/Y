#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>

void print_indent(int depth);
char* cstr_skip_whitespace(char** s, char* end, int expect_more);

#endif //HELPERS_H
