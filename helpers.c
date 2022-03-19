#include "helpers.h"

void print_indent(int depth) {
    
    for(int i = 0; i < depth; i++) {
    
        printf("    ");
    }
}

char* cstr_skip_whitespace(char** s, char* end, int expect_more) {

    for(; (*s) != end; (*s)++) if(**s > 0x20) return 0;

    return expect_more ? "Hit the end of a cstr while skipping whitespace" : 0;
}

void skip_whitespace(FILE* in_file) {
    
    char c = 0;
    fpos_t last_pos;
    
    while(1) {

        fgetpos(in_file, &last_pos);

        if(fread(&c, 1, 1, in_file) != 1) return;

        if(c > 0x20) {
        
            fsetpos(in_file, &last_pos);
            
            return;
        }
    }
}


