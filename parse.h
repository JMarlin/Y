#ifndef PARSE_H
#define PARSE_H

#include "ast.h"
#include "scanner.h"
#include <stdio.h>

char* StringLiteral_tryParse(Scanner scanner, ASTNode** node, int level);

char* Value_tryParse(Scanner scanner, ASTNode** node, int level);

char* Symbol_tryParse(Scanner scanner, ASTNode** node, int level);

char* Operator_tryParse(Scanner scanner, ASTNode** node, int level);

char* Parameter_tryParse(Scanner scanner, ASTNode** node, int level);

char* ParameterList_tryParse(Scanner scanner, ASTNode** node, int level);

char* Lambda_tryParse(Scanner scanner, ASTNode** node, int level);

char* Invocation_tryParse(Scanner scanner, ASTNode** node, int level);

char* Expression_tryParse(Scanner scanner, ASTNode** node, int level);

char* ExpressionStatement_tryParse(Scanner scanner, ASTNode** node, int level);

char* Declaration_tryParse(Scanner scanner, ASTNode** node, int level);

char* Statement_tryParse(Scanner scanner, ASTNode** node, int level);

char* Module_tryParse(Scanner scanner, ASTNode** node, int level);

#endif //PARSE_H
