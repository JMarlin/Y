#include "parse.h"
#include "helpers.h"
#include "scanner.h"
#include "debug.h"
#include <stdlib.h>

int characterIsDecimalNumeric(char c) {

    return c >= '0' && c <= '9';
}

int characterToDecimalDigit(char c) {

    return c - '0';
}

char* NumberLiteral_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse a number\n");

    ScannerBegin(scanner);

    char* error;
    long int value = 0;
    ScanResult sr = { 0 };

    ScannerSkipWhitespace(scanner);

    int i;
    for(i = 0; ; i++) {

        if(!ScannerCheckpoint(scanner)) {

            ScannerRollbackFull(scanner);
            
            return "Failed to get file position";
        }

        sr = ScannerGetNext(scanner);
        if(sr.err) return "Encountered end of file inside of number";
        if(!characterIsDecimalNumeric(sr.val)) break;
        value = value * 10 + characterToDecimalDigit(sr.val);
    }

    if(i == 0) {

        ScannerRollbackLast(scanner);
        
        return "Number had no digits";
    }

    if((error = ASTNode_create(node, NumberLiteral, 0, 1)) != 0) {

        ScannerRollbackFull(scanner);

        return "Failed to allocate space for a number literal";
    }

    (*node)->NLN_NUMBER = (void*)value;

    ScannerRollbackLast(scanner);

    return 0;
}

char* Value_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse a value\n");

    char* error;

    if(NumberLiteral_tryParse(scanner, node, level + 1) == 0) return 0;
    if(StringLiteral_tryParse(scanner, node, level + 1) == 0) return 0;
    if((error = Symbol_tryParse(scanner, node, level + 1)) == 0) return 0;

    return error;
}

char* StringLiteral_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse a string literal\n");

    ScannerBegin(scanner);

    char* error;
    ScanResult sr = { 0 };
    String* string;

    ScannerSkipWhitespace(scanner);
    
    if(!ScannerNextIs(scanner, '"'))
        return "String literal did not begin with double-quotes";

    string = String_new(0);

    if(!string) {

        ScannerRollbackFull(scanner);

        return "Failed to allocate string for a string literal";
    }

    int ignore_quote = 0;

    while(1) {

        sr = ScannerGetNextStrict(scanner);

        if(sr.err) {

            String_cleanUp(string);

            return "Encountered end of file inside of string";
        }

	    if(sr.val == '"' && !ignore_quote)
            break;

        ignore_quote = 0;

#warning TODO: Implement real escape character handling
/*
	    if(sr.val == '\\')
            ignore_quote = 1;
        else
*/
            String_appendChar(string, sr.val);
    }

    if((error = ASTNode_create(node, StringLiteral, 0, 1)) != 0) {

        ScannerRollbackFull(scanner);
        String_cleanUp(string);

    	return "Failed to allocate space for a string literal";
    }

    (*node)->SLN_STRING = string;

    return 0;
}

char* Symbol_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse a symbol\n");

    char* error;
    ScanResult sr = { 0 };

    ScannerBegin(scanner);

    String* symbol_text = String_new(0);

    if(!symbol_text) return "Unable to allocate String for symbol text";

    ScannerSkipWhitespace(scanner);

    for(int i = 0; ; i++) {

        if(!ScannerCheckpoint(scanner)) {

            ScannerRollbackFull(scanner);
            String_cleanUp(symbol_text);
            
            return "Failed to get file position";
        }

        sr = ScannerGetNext(scanner);

        if(sr.err) break;

        if(
            (sr.val >= 'a' && sr.val <= 'z') ||
            (sr.val >= 'A' && sr.val <= 'Z') ||
            (!!i && (sr.val > 0 && sr.val >= '0' && sr.val <= '9')) ||
            (sr.val == '_')
        ) {

            error = String_appendChar(symbol_text, sr.val);

            if(error != 0) {
        
                ScannerRollbackFull(scanner);
                String_cleanUp(symbol_text);

                return error;
            }

            continue;
        }

        if(i == 0) {

            ScannerRollbackFull(scanner);
            String_cleanUp(symbol_text);

            return "Symbol did not begin with a valid character";
        }

        ScannerRollbackLast(scanner);

        break;
    }

    error = ASTNode_create(node, Symbol, 0, 1);

    if(*node == 0) {

        ScannerRollbackFull(scanner);

        String_cleanUp(symbol_text);

        return "Couldn't allocate memory for ast symbol";
    }

    (*node)->SN_TEXT = (void*)symbol_text;

    return 0;
}

char* Operator_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse an operator\n");

    ScannerBegin(scanner);
    ScannerSkipWhitespace(scanner);

    ASTNode* left_expr;

    char* left_error = Value_tryParse(scanner, &left_expr, level + 1);

    if(left_error != 0) {
    
        ScannerRollbackFull(scanner);

        return left_error;
    }

    ScannerSkipWhitespace(scanner);

    ScanResult sr = { 0 };

    if(!ScannerCheckpoint(scanner)) {

        ASTNode_cleanUp(left_expr);
        
        return "Failed to get file position";
    }

    sr = ScannerGetNextStrict(scanner);

    if(sr.err) return "Unexpected EOF attempting to read operator";

    ASTOperatorType op_type =
        sr.val == '+' ? OpAdd      :
        sr.val == '-' ? OpSubtract :
        sr.val == '*' ? OpMultiply :
        sr.val == '/' ? OpDivide   :
                        OpInvalid  ;

    if(op_type == OpInvalid) {

                //printf("Invalid op '%c'\n", sr.val);

        ScannerRollbackLast(scanner);

        *node = left_expr;

        return 0;
    }

    ScannerSkipWhitespace(scanner);
    
    ASTNode* right_expr;

    char* right_error = Value_tryParse(scanner, &right_expr, level + 1);

    if(right_error != 0) {
    
        ScannerRollbackFull(scanner);
        ASTNode_cleanUp(left_expr);

        return right_error;
    }

    char* error = ASTNode_create(node, Operator, 2, 1);

    if(error != 0) {

        ScannerRollbackFull(scanner);
        ASTNode_cleanUp(left_expr);
        ASTNode_cleanUp(right_expr);

        return "Failed to allocate memory for an operator node";
    }

    (*node)->ON_LEFT_EXPR = left_expr;
    (*node)->ON_RIGHT_EXPR = right_expr;
    (*node)->ON_OPERATOR = (void*)op_type;

    return 0;
}

char* Parameter_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse a parameter\n");
    
    ScanResult sr = { 0 };

    ScannerBegin(scanner);
    ScannerSkipWhitespace(scanner);

    if(!ScannerNextIsStr(scanner, "var"))
        return "Parameter declaration did not begin with 'var' keyword";

    sr = ScannerGetNextStrict(scanner);

    if(sr.val > 0x20) {

        ScannerRollbackFull(scanner);

        return "Parameter declaration 'var' keyword not followed by whitespace";
    }
    
    ASTNode* symbol;

    ScannerSkipWhitespace(scanner);

    char* error = Symbol_tryParse(scanner, &symbol, level + 1);

    if(error != 0) return error;

    error = ASTNode_create(node, Parameter, 1, 0);

    if(error != 0) {
    
        ScannerRollbackFull(scanner);
        ASTNode_cleanUp(symbol);

        return "Unable to allocate space for new parameter declaration node";
    }

    (*node)->PN_SYMBOL = symbol;

    return 0;
}

char* ParameterList_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse a parameter list\n");

    ScanResult sr = { 0 };

    ScannerBegin(scanner);
    ScannerSkipWhitespace(scanner);

    if(!ScannerNextIs(scanner, '(')) return "Expected '(' at start of parameter list";

    int paramDeclarationsCapacity = 0;
    int paramDeclarationsCount = 0;
    char* error = 0;
    ASTNode** paramDeclarations = 0;

    while(1) {
    
        ASTNode* parameter;

        error = Parameter_tryParse(scanner, &parameter, level + 1);

        if(error) {

            if(paramDeclarationsCount > 0) free(paramDeclarations);

            ScannerRollbackFull(scanner);

            return error;
        }

        if(paramDeclarationsCapacity == paramDeclarationsCount) {

            int new_size = paramDeclarationsCapacity == 0
                ? 1
                : (2 * paramDeclarationsCapacity);
    
            paramDeclarationsCapacity = new_size;
            ASTNode** newParametersPtr = (ASTNode**)realloc(paramDeclarations, sizeof(ASTNode*) * paramDeclarationsCapacity );

            if(newParametersPtr != 0) {

                paramDeclarations = newParametersPtr;
            } else {

                if(paramDeclarationsCount > 0) free(paramDeclarations);

                ScannerRollbackFull(scanner);

                return "Failed to allocate memory for parameter list";
            }
        }

        paramDeclarations[paramDeclarationsCount++] = parameter;

        ScannerSkipWhitespace(scanner);
        sr = ScannerGetNextStrict(scanner);

        if(sr.err) {

            if(paramDeclarationsCount > 0) free(paramDeclarations);

            return "Unexpected EOF in parameter list";
        }

        if(sr.val != ',') break;
    }

    if(sr.val != ')') {

        if(paramDeclarationsCount > 0) free(paramDeclarations);

        ScannerRollbackFull(scanner);

        return "Expected a closing parenthesis at the end of parameter list";
    }

    error = ASTNode_create(node, ParameterList, 0, 0);

    if(error != 0) {

        if(paramDeclarationsCount > 0) free(paramDeclarations);

        ScannerRollbackFull(scanner);

        return "Failed to allocate memory for parameter list";
    }

    (*node)->childCount = paramDeclarationsCount;
    (*node)->children = paramDeclarations;
    
    return 0;
}

char* Lambda_tryParse(Scanner scanner, ASTNode** node, int level) {

    static int lambda_id = 0;

    DEBUG_INDENT_PRINT(level, "Trying to parse a lambda\n");

    ASTNode* parameterList;

    ScannerBegin(scanner);

    char* pl_error = ParameterList_tryParse(scanner, &parameterList, level + 1);

    if(pl_error != 0) {
    
        ScannerRollbackFull(scanner);

        return pl_error;
    }

    ScannerSkipWhitespace(scanner);

    if(!ScannerNextIsStr(scanner, "=>")) {

        ASTNode_cleanUp(parameterList);

        return "Expected '=>' following lambda parameter list";
    }

    ScannerSkipWhitespace(scanner);

    ASTNode* expression;

    char* expression_error = Expression_tryParse(scanner, &expression, level + 1);

    if(expression_error != 0)  {

        ASTNode_cleanUp(parameterList);

        ScannerRollbackFull(scanner);

        return expression_error;
    }

    char* error = ASTNode_create(node, Lambda, 2, 1);

    if(error != 0)  {

        ASTNode_cleanUp(parameterList);
        ASTNode_cleanUp(expression);

        ScannerRollbackFull(scanner);

        return expression_error;
    }

    (*node)->LN_PARAMS = parameterList;
    (*node)->LN_EXPR = expression;
    (*node)->attributes[0] = (void*)(size_t)(lambda_id++);

    return 0;
}

char* ArgumentList_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse an argument list\n");

    VoidList child_list;

    ScannerBegin(scanner);
    VoidList_init(&child_list);

    ScanResult sr = { 0 };
    int expect_next = 0;
    char* error;
    ASTNode* arg_expression;

    if(!ScannerNextIs(scanner, '(')) {

        ScannerRollbackFull(scanner);

    	return "Argument list did not begin with an opening paren";
    }

    while(1) {

    	ScannerSkipWhitespace(scanner);

    	error = Expression_tryParse(scanner, &arg_expression, level + 1);

    	if(error != 0 && expect_next) {

            VoidList_cleanUp(&child_list);

    	    return "Expected argument following comma in argument list";
    	}

    	if(error == 0) {

            error = VoidList_add(&child_list, arg_expression);

	        if(error != 0) {

                VoidList_cleanUp(&child_list);

                return error;
	        }
        }

    	ScannerSkipWhitespace(scanner);
        sr = ScannerGetNextStrict(scanner);
    
        if(sr.err) {

            //TODO: We should clean up the individual nodes as well
            VoidList_cleanUp(&child_list);

    	    return "Unexpected EOF reading argument list";
	    }

        if(sr.val == ')') break;
        if(sr.val == ',') expect_next = 1;
    }

    error = ASTNode_create(node, ArgumentList, 0, 0);

    if(error != 0) {
    
        VoidList_cleanUp(&child_list);
        ScannerRollbackFull(scanner);
        
    	return "Failed to allocate memory for an argument list node";
    }

    (*node)->childCount = child_list.count;
    (*node)->children = (ASTNode**)child_list.data;

    return 0;
}

char* Invocation_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse an invocation\n");

    ScanResult sr = { 0 };
    char* error;
    ASTNode* symbol;

    ScannerBegin(scanner);

    error = Symbol_tryParse(scanner, &symbol, level + 1);

    if(error != 0) {

        ScannerRollbackFull(scanner);

    	return error;
    }

    ScannerSkipWhitespace(scanner);

    ASTNode* arguments;
    error = ArgumentList_tryParse(scanner, &arguments, level + 1);

    if(error != 0) {

    	ASTNode_cleanUp(symbol);
        ScannerRollbackFull(scanner);

    	return error;
    }

    error = ASTNode_create(node, Invocation, 2, 0);

    if(error != 0) {
    
    	ASTNode_cleanUp(symbol);
    	ASTNode_cleanUp(arguments);
        ScannerRollbackFull(scanner);
        
    	return "Failed to allocate memory for an invocation node";
    }

    (*node)->IN_SYMBOL = symbol;
    (*node)->IN_ARGS = arguments;

    return 0;
}

char* Expression_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse an expression\n");

    char* error;
    
    if(Lambda_tryParse(scanner, node, level + 1) == 0) return 0;
    if((error = Invocation_tryParse(scanner, node, level + 1)) == 0) return 0;
    if((error = Operator_tryParse(scanner, node, level + 1)) == 0) return 0;

    return error;
}

char* Declaration_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse a declaration\n");
    
    ScanResult sr = { 0 };

    ScannerBegin(scanner);
    ScannerSkipWhitespace(scanner);

    if(!ScannerNextIsStr(scanner, "var"))
        return "Declaration statement did not begin with 'var' keyword";

    sr = ScannerGetNextStrict(scanner);

    if(sr.err || sr.val > 0x20) 
        return "Declaration statement did not begin with 'var' keyword";
    
    ASTNode* lvalue;

    ScannerSkipWhitespace(scanner);

    char* error = Symbol_tryParse(scanner, &lvalue, level + 1);

    if(error != 0) return error;

    ScannerSkipWhitespace(scanner);

    char eq;

    ASTNode* rvalue = 0;

    sr = ScannerGetNext(scanner);

    if(!sr.err && sr.val == '=') {

        error = Expression_tryParse(scanner, &rvalue, level + 1);

        if(error != 0) {

            ScannerRollbackFull(scanner);
            ASTNode_cleanUp(lvalue);

            return error;
        }
    }

    ScannerSkipWhitespace(scanner);

    if(!ScannerNextIs(scanner, ';')) {

        ASTNode_cleanUp(lvalue);

        if(rvalue != 0) ASTNode_cleanUp(rvalue);

        return "No semicolon following logical end of declaration statement";
    }

    error = ASTNode_create(node, Declaration, 2, 0);

    if(error != 0) {
    
        ScannerRollbackFull(scanner);
        ASTNode_cleanUp(lvalue);
        if(rvalue != 0) ASTNode_cleanUp(rvalue);

        return "Unable to allocate space for new declaration statement node";
    }

    (*node)->DN_SYMBOL = lvalue;
    (*node)->DN_INITIALIZER = rvalue;

    return 0;
}

char* ExpressionStatement_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse an expression statement\n");

    char* error;

    ScannerBegin(scanner);

    if((error = Expression_tryParse(scanner, node, level + 1)) != 0) return error;

    ScannerSkipWhitespace(scanner);

    if(!ScannerNextIs(scanner, ';')) return "Expression statement did not end in ';'\n";

    return 0;
}

char* Statement_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse a statement\n");

    ScannerSkipWhitespace(scanner);

    char* error;

    if((error = Declaration_tryParse(scanner, node, level + 1)) == 0) return 0;
    if((error = ExpressionStatement_tryParse(scanner, node, level + 1)) == 0) return 0;

    return error;
}

char* Module_tryParse(Scanner scanner, ASTNode** node, int level) {

    DEBUG_INDENT_PRINT(level, "Trying to parse a module\n");

    int statementCapacity = 0;
    char* inner_error = 0;
    ASTNode* new_statement;

    inner_error = ASTNode_create(node, Module, 0, 0);

    if(inner_error != 0) return "Unable to allocate memory for a module node";

    while((!ScannerAtEnd(scanner)) && ((inner_error = Statement_tryParse(scanner, &new_statement, level + 1)) == 0)) {
        
        if(statementCapacity < ((*node)->childCount + 1)) {

            int nextCapacity = (statementCapacity == 0) ? 1 : (statementCapacity * 2);
        
            (*node)->children = (ASTNode**)realloc((*node)->children, nextCapacity * sizeof(ASTNode*));

            if((*node)->children == 0) {

                inner_error = "Failed to allocate statement memory when constructing module node";

                break;
            }

            statementCapacity = nextCapacity;
        }

        (*node)->children[(*node)->childCount++] = new_statement;

        ScannerSkipWhitespace(scanner);
    }

    return inner_error;
}


