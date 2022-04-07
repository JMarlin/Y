#include "parse.h"
#include "helpers.h"
#include <stdlib.h>

char* Symbol_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    //printf("Trying to parse a symbol\n");

    fpos_t original_position, previous_position;
    char* error;
    char c = 0;
    String* symbol_text = String_new(0);

    if(!symbol_text) return "Unable to allocate String for symbol text";

    if(fgetpos(in_file, &original_position) != 0) return "Failed to get file position";

    skip_whitespace(in_file);

    for(int i = 0; ; i++) {

        if(fgetpos(in_file, &previous_position) != 0) {

            fsetpos(in_file, &original_position);

            String_cleanUp(symbol_text);
            
            return "Failed to get file position";
        }

        if(fread(&c, 1, 1, in_file) != 1) break;

        if(
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (i > 0 && c >= '0' && c <= '9') ||
            (c == '_')
        ) {

            error = String_appendChar(symbol_text, c);

            if(error != 0) {
        
                fsetpos(in_file, &original_position);

                String_cleanUp(symbol_text);

                return error;
            }

            continue;
        }

        if(i == 0) {

            fsetpos(in_file, &original_position);

            String_cleanUp(symbol_text);

            return "Symbol did not begin with a valid character";
        }

        fsetpos(in_file, &previous_position);

        break;
    }

    error = ASTNode_create(node, Symbol, 0, 1, sizeof(ASTSymbolNode));

    if(*node == 0) {

        fsetpos(in_file, &original_position);

        String_cleanUp(symbol_text);

        return "Couldn't allocate memory for ast symbol";
    }

    (*node)->SN_TEXT = (void*)symbol_text;

    return 0;
}

char* Operator_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    //printf("Trying to parse an operator\n");

    fpos_t original_position, before_op_read_position;

    if(fgetpos(in_file, &original_position) != 0) return "Failed to get file position";

    skip_whitespace(in_file);

    ASTNode* left_expr;

    char* left_error = Symbol_tryParse(in_file, &left_expr);

    if(left_error != 0) {
    
        fsetpos(in_file, &original_position);

        return left_error;
    }

    skip_whitespace(in_file);

    char c;

    if(fgetpos(in_file, &before_op_read_position) != 0) {

        ASTNode_cleanUp(left_expr);
        
        return "Failed to get file position";
    }

    if(fread(&c, 1, 1, in_file) != 1) return "Unexpected EOF attempting to read operator";

    ASTOperatorType op_type =
        c == '+' ? OpAdd      :
        c == '-' ? OpSubtract :
        c == '*' ? OpMultiply :
        c == '/' ? OpDivide   :
                   OpInvalid  ;

    if(op_type == OpInvalid) {

        //TEMP
        //printf("Invalid op '%c'\n", c);

        fsetpos(in_file, &before_op_read_position);

        *node = left_expr;

        return 0;
    }

    skip_whitespace(in_file);
    
    ASTNode* right_expr;

    char* right_error = Symbol_tryParse(in_file, &right_expr);

    if(right_error != 0) {
    
        fsetpos(in_file, &original_position);

        ASTNode_cleanUp(left_expr);

        return right_error;
    }

    char* error = ASTNode_create(node, Operator, 2, 0, sizeof(ASTOperatorNode));

    if(error != 0) {

        fsetpos(in_file, &original_position);

        ASTNode_cleanUp(left_expr);
        ASTNode_cleanUp(right_expr);

        return "Failed to allocate memory for an operator node";
    }

    (*node)->ON_LEFT_EXPR = left_expr;
    (*node)->ON_RIGHT_EXPR = right_expr;

    return 0;
}

char* Parameter_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    //printf("Trying to parse a parameter\n");

    fpos_t original_position;

    if(fgetpos(in_file, &original_position) != 0) return "Failed to get file position";

    skip_whitespace(in_file);

    //TODO: actually declare a type
    char kw_buf[4];
    const char* var_keyword = "var";

    if(fread(kw_buf, 1, 4, in_file) != 4) return "Unexpected EOF in parameter declaration";

    for(int i = 0; i < 3; i++) if(kw_buf[i] != var_keyword[i]) {
    
        fsetpos(in_file, &original_position);

        return "Parameter declaration did not begin with 'var' keyword";
    }

    if(kw_buf[3] > 0x20) {

        fsetpos(in_file, &original_position);

        return "Parameter declaration 'var' keyword not followed by whitespace";
    }
    
    ASTNode* symbol;

    skip_whitespace(in_file);

    char* error = Symbol_tryParse(in_file, &symbol);

    if(error != 0) return error;

    error = ASTNode_create(node, Parameter, 1, 0, sizeof(ASTParameterNode));

    if(error != 0) {
    
        fsetpos(in_file, &original_position);

        ASTNode_cleanUp(symbol);

        return "Unable to allocate space for new parameter declaration node";
    }

    (*node)->PN_SYMBOL = symbol;

    return 0;
}

char* ParameterList_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    //printf("Trying to parse a parameter list\n");

    fpos_t original_position;
    char* error;

    if(fgetpos(in_file, &original_position) != 0) return "Failed to get file position";

    skip_whitespace(in_file);

    char c;

    if(fread(&c, 1, 1, in_file) != 1) return "Unexpected EOF starting argument list";

    if(c != '(') return "Expected '(' at start of argument list";

    int paramDeclarationsCapacity = 0;
    int paramDeclarationsCount = 0;

    ASTNode** paramDeclarations = 0;

    while(1) {
    
        ASTNode* parameter;

        error = Parameter_tryParse(in_file, &parameter);

        if(error) {

            if(paramDeclarationsCount > 0) free(paramDeclarations);

            fsetpos(in_file, &original_position);

            return error;
        }

        if(paramDeclarationsCapacity == paramDeclarationsCount) {

            int new_size = paramDeclarationsCapacity == 0
                ? 1
                : (2 * paramDeclarationsCapacity);
    
            ASTNode** newParametersPtr = (ASTNode**)malloc(sizeof(ASTNode*) * new_size );

            if(newParametersPtr != 0) {

                paramDeclarations = newParametersPtr;
            } else {

                if(paramDeclarationsCount > 0) free(paramDeclarations);

                fsetpos(in_file, &original_position);

                return "Failed to allocate memory for parameter list";
            }
        }

        paramDeclarations[paramDeclarationsCount++] = parameter;

        skip_whitespace(in_file);

        if(fread(&c, 1, 1, in_file) != 1) {

            if(paramDeclarationsCount > 0) free(paramDeclarations);

            fsetpos(in_file, &original_position);

            return "Unexpected EOF in parameter list";
        }

        if(c != ',') break;
    }

    if(c != ')') {

        if(paramDeclarationsCount > 0) free(paramDeclarations);

        fsetpos(in_file, &original_position);

        return "Expected a closing parenthesis at the end of parameter list";
    }

    error = ASTNode_create(node, ParameterList, 0, 0, sizeof(ASTParameterListNode));

    if(error != 0) {

        if(paramDeclarationsCount > 0) free(paramDeclarations);

        fsetpos(in_file, &original_position);

        return "Failed to allocate memory for parameter list";
    }

    (*node)->childCount = paramDeclarationsCount;
    (*node)->children = paramDeclarations;
    
    return 0;
}

char* Lambda_tryParse(FILE* in_file, ASTNode** node) {

    static int lambda_id = 0;

    //TEMP
    //printf("Trying to parse a lambda\n");

    ASTNode* parameterList;
    fpos_t original_position;

    if(fgetpos(in_file, &original_position) != 0) return "Failed to get file position";

    char* pl_error = ParameterList_tryParse(in_file, &parameterList);

    if(pl_error != 0) {
    
        fsetpos(in_file, &original_position);

        return pl_error;
    }

    skip_whitespace(in_file);

    char arrow_buf[2];

    if(fread(arrow_buf, 1, 2, in_file) != 2) {

        ASTNode_cleanUp(parameterList);

        fsetpos(in_file, &original_position);

        return "Unexpected EOF after lambda expression argument list";
    }

    if(arrow_buf[0] != '=' || arrow_buf[1] != '>')  {

        ASTNode_cleanUp(parameterList);

        fsetpos(in_file, &original_position);

        return "Expected '=>' following lambda argument list";
    }

    skip_whitespace(in_file);

    ASTNode* expression;

    char* expression_error = Expression_tryParse(in_file, &expression);

    if(expression_error != 0)  {

        ASTNode_cleanUp(parameterList);

        fsetpos(in_file, &original_position);

        return expression_error;
    }

    char* error = ASTNode_create(node, Lambda, 2, 1, sizeof(ASTLambdaNode));

    if(error != 0)  {

        ASTNode_cleanUp(parameterList);
        ASTNode_cleanUp(expression);

        fsetpos(in_file, &original_position);

        return expression_error;
    }

    (*node)->LN_PARAMS = parameterList;
    (*node)->LN_EXPR = expression;
    (*node)->attributes[0] = (void*)(size_t)(lambda_id++);

    return 0;
}

char* Invocation_tryParse(FILE* in_file, ASTNode** node) {

    char c;
    char* error;
    VoidList child_list;
    fpos_t original_position;
    ASTNode* symbol;

    if(fgetpos(in_file, &original_position) != 0) return "Failed to get file position";

    error = Symbol_tryParse(in_file, &symbol);

    if(error != 0) {

        fsetpos(in_file, &original_position);

	return error;
    }

    if((error = VoidList_add(&child_list, symbol) != 0)) {
        
        fsetpos(in_file, &original_position);

        ASTNode_cleanUp(symbol);

        return error;
    }

    skip_whitespace(in_file);

    if(fread(&c, 1, 1, in_file) != 1) return "Unexpected EOF attempting to read invocation";

    if(c != '(') {

        fsetpos(in_file, &original_position);

	return "Expected opening paren following symbol in invocation";
    }

    int expect_next = 0;

    while(1) {

	skip_whitespace(in_file);

	error = Expression_tryParse(in_file, &arg_expression);

	if(error != 0 && expect_next) {

            VoidList_cleanUp(child_list);
	    ASTNode_cleanUp(symbol);

	    return "Expected argument following comma in invocation";
	}

	if(error == 0) {

            error = VoidList_add(child_list, arg_expression);

	    if(error != 0) {

		VoidList_cleanUp(child_list);
		ASTNode_cleanUp(symbol);

		return error;
	    }
        }

	skip_whitespace(in_file);
    
        if(fread(&c, 1, 1, in_file) != 1) {

	    //TODO: We should clean up the individual nodes as well
	    VoidList_cleanUp(child_list);
	    ASTNode_cleanUp(symbol);

	    return "Unexpected EOF reading invocation param list";
	}

	if(c == ')') break;

        if(c == ',') expect_next = 1;
    }

    error = ASTNode_create(node, Invocation, argExpressionCount + 1, 1,
		    sizeof(ASTInvocationNode));

    if(error != 0) {
    
        VoidList_cleanUp(child_list);
	ASTNode_cleanUp(symbol);
        
	return "Failed to allocate memory for an invocation node";
    }

    (*node)->childCount = child_list.count;
    (*node)->children = (ASTNode**)child_list.data;

    return 0;
}

char* Expression_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    //printf("Trying to parse an expression\n");

    char* error;
    
    if(Lambda_tryParse(in_file, node) == 0) return 0;
    if((error = Operator_tryParse(in_file, node)) == 0) return 0;
    if((error = Invocation_tryParse(in_file, node)) == 0) return 0;

    return error;
}

char* Declaration_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    //printf("Trying to parse a declaration\n");

    fpos_t original_position;

    if(fgetpos(in_file, &original_position) != 0) return "Failed to get file position";

    skip_whitespace(in_file);

    char kw_buf[4];
    const char* var_keyword = "var";

    if(fread(kw_buf, 1, 4, in_file) != 4) return "Unexpected EOF in variable declaration";

    for(int i = 0; i < 3; i++) if(kw_buf[i] != var_keyword[i]) {
    
        fsetpos(in_file, &original_position);

        return "Declaration statement did not begin with 'var' keyword";
    }

    if(kw_buf[3] > 0x20) {

        fsetpos(in_file, &original_position);

        return "Declaration statement did not begin with 'var' keyword";
    }
    
    ASTNode* lvalue;

    skip_whitespace(in_file);

    char* error = Symbol_tryParse(in_file, &lvalue);

    if(error != 0) return error;

    skip_whitespace(in_file);

    char eq;

    ASTNode* rvalue = 0;

    if(fread(&eq, 1, 1, in_file) == 1 && eq == '=') {

        error = Expression_tryParse(in_file, &rvalue);

        if(error != 0) {

            fsetpos(in_file, &original_position);
            ASTNode_cleanUp(lvalue);

            return error;
        }
    }

    skip_whitespace(in_file);

    char sc;

    if(fread(&sc, 1, 1, in_file) != 1 || sc != ';') {

        fsetpos(in_file, &original_position);
        ASTNode_cleanUp(lvalue);

        if(rvalue != 0) ASTNode_cleanUp(rvalue);

        return "No semicolon following logical end of declaration statement";
    }

    error = ASTNode_create(node, Declaration, 2, 0, sizeof(ASTDeclarationNode));

    if(error != 0) {
    
        fsetpos(in_file, &original_position);
        ASTNode_cleanUp(lvalue);
        if(rvalue != 0) ASTNode_cleanUp(rvalue);

        return "Unable to allocate space for new declaration statement node";
    }

    (*node)->DN_SYMBOL = lvalue;
    (*node)->DN_INITIALIZER = rvalue;

    return 0;
}

char* Statement_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    //printf("Trying to parse a statement\n");

    skip_whitespace(in_file);

    char* error;

    if((error = Declaration_tryParse(in_file, node)) == 0) return 0;
    if((error = Expression_tryParse(in_file, node)) == 0) return 0;

    return error;
}

char* Module_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    //printf("Trying to parse a module\n");

    int statementCapacity = 0;
    char* inner_error = 0;
    ASTNode* new_statement;

    inner_error = ASTNode_create(node, Module, 0, 0, sizeof(ASTModuleNode));

    if(inner_error != 0) return "Unable to allocate memory for a module node";

    while((!feof(in_file)) && ((inner_error = Statement_tryParse(in_file, &new_statement)) == 0)) {
        
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

        skip_whitespace(in_file);
    }

    return inner_error;
}


