#ifndef PARSE_H
#define PARSE_H

#include "ast.h"
#include <stdio.h>

char* StringLiteral_tryParse(FILE* in_file, ASTNode** node);

char* Value_tryParse(FILE* in_file, ASTNode** node);

char* Symbol_tryParse(FILE* in_file, ASTNode** node);

char* Operator_tryParse(FILE* in_file, ASTNode** node);

char* Parameter_tryParse(FILE* in_file, ASTNode** node);

char* ParameterList_tryParse(FILE* in_file, ASTNode** node);

char* Lambda_tryParse(FILE* in_file, ASTNode** node);

char* Invocation_tryParse(FILE* in_file, ASTNode** node);

char* Expression_tryParse(FILE* in_file, ASTNode** node);

char* Declaration_tryParse(FILE* in_file, ASTNode** node);

char* Statement_tryParse(FILE* in_file, ASTNode** node);

char* Module_tryParse(FILE* in_file, ASTNode** node);

#endif //PARSE_H
