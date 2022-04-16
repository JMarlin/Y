#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parse.h"
#include "template.h"
#include "ctemplate.h"

#define MODE_WRITE_C  0
#define MODE_DUMP_AST 1

int main(int argc, char* argv[]) {

    if(argc < 2) {

        printf("Usage: yc <in_file.y> [-o out_file | -t out_file.c]\n");

        return 0;
    }

    int mode = MODE_WRITE_C;
    char* in_name = 0;
    char* out_name = "out.c";

    for(int i = 1; i < argc; i++) {

        if(argc >= (i + 2) && argv[i][0] == '-' && argv[i][1] == 'o' && argv[i][2] == 0) {

            out_name = argv[++i];
    
            continue;
        }

        if(argv[i][0] == '-' && argv[i][1] == 'a' && argv[i][2] == 0) {

            mode = MODE_DUMP_AST;

            continue;
        }

        in_name = argv[i];
    }

    if(in_name == 0) {

        printf("No input file specified!");
        
        return 0;
    }

    FILE* in_file = fopen(in_name, "r");

    if(in_file == 0) {

        printf("Unable to open input file %s\n", in_name);

        return 0;
    }
    
    ASTNode* module_ast;
    Scanner scanner = NewScanner(in_file);

    char* error_message = Module_tryParse(scanner, &module_ast, 0);

    if(error_message) {
        
        printf("Compilation failed: %s\n", error_message);

        ASTNode_cleanUp(module_ast); 

        return 1;
    }

    if(mode == MODE_WRITE_C) {

        //TODO: Actually parse command line args as described
        FILE* out_file = fopen(out_name, "w");

        if(out_file == 0) {

            printf("Unable to open output file %s\n", out_name);

            ASTNode_cleanUp(module_ast);

            return 0;
        }

        error_message = ASTNode_writeOut(out_file, &CTemplateConfig, module_ast);

        if(error_message != 0)
            printf("Writing out failed: %s\n", error_message);

        fclose(out_file);
    }

    if(mode == MODE_DUMP_AST) {

        ASTNode_print(module_ast, 0);
    }

    ASTNode_cleanUp(module_ast);

    return 0;
}
