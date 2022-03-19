#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parse.h"
#include "template.h"
#include "ctemplate.h"

int main(int argc, char* argv[]) {

    if(argc < 2) {

        printf("Usage: yc <in_file.y> [-o out_file | -t out_file.c]\n");

        return 0;
    }

    char* in_name = argv[1];

    FILE* in_file = fopen(in_name, "r");

    ASTNode* module_ast;

    char* error_message = Module_tryParse(in_file, &module_ast);

    if(error_message) {
        
        printf("Compilation failed: %s\n", error_message);

        ASTNode_cleanUp(module_ast); 

        return 1;
    }

    //TODO: Actually parse command line args as described
    FILE* out_file = fopen(argv[2], "w");

    error_message = ASTNode_writeOut(out_file, &CTemplateConfig, module_ast);

    if(error_message != 0)
        printf("Writing out failed: %s\n", error_message);

    fclose(out_file);

    ASTNode_cleanUp(module_ast);

    return 0;
}
