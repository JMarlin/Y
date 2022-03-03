#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    Module,
    Statement,
    Expression,
    Parameter,
    ParameterList,
    Symbol,
    Operator,
    Literal,
    ASTNodeTypeCount
} ASTNodeType;

typedef struct ASTNode_s {
    ASTNodeType type;
} ASTNode;

typedef void (*ASTNodePrinter)(struct ASTNode_s*);
typedef void (*ASTNodeCleaner)(struct ASTNode_s*);

#define AN_METHODS_DECL(n) \
    void AST ## n ## Node_print(struct ASTNode_s*); \
    void AST ## n ## Node_cleanUp(struct ASTNode_s*)

AN_METHODS_DECL(Module);
AN_METHODS_DECL(Statement);
AN_METHODS_DECL(Expression);
AN_METHODS_DECL(Parameter);
AN_METHODS_DECL(ParameterList);
AN_METHODS_DECL(Symbol);
AN_METHODS_DECL(Declaration);
AN_METHODS_DECL(Operator);
AN_METHODS_DECL(Literal);


#define AN_METHODS_STRUCT(n) \
    (ASTNodeMethods){ AST ## n ## Node_print, AST ## n ## Node_cleanUp }

typedef struct ASTNodeMethods_s {
    ASTNodePrinter print;
    ASTNodeCleaner cleanUp;
} ASTNodeMethods;

const ASTNodeMethods ASTNodeMethodsFor[] = {
    AN_METHODS_DECL(Module),
    AN_METHODS_DECL(Statement),
    AN_METHODS_DECL(Expression),
    AN_METHODS_DECL(Parameter),
    AN_METHODS_DECL(ParameterList),
    AN_METHODS_DECL(Symbol),
    AN_METHODS_DECL(Declaration),
    AN_METHODS_DECL(Operator),
    AN_METHODS_DECL(Literal)
};

typedef struct String_s {
    uint32_t capacity;
    uint32_t length;
    char* data;
} String;

typedef struct ASTStatementNode_s {
    ASTNodeType type;
    ASTStatementType statementType;
} ASTStatementNode;

typedef struct ASTSymbol_s {
    String* text;
} ASTSymbol;

typedef struct ASTParameterNode_s {
    ASTNodeType type;
    ASTSymbol* symbol;
} ASTParameterNode;

typedef struct ASTParamterListNode_s {
    ASTNodeType type;
    int count;
    ASTParameterNode** parameters;
} ASTParameterListNode;

typedef enum {
    OpAdd,
    OpSubtract,
    OpMultiply,
    OpDivide,
    OpInvalid
} ASTOperatorType;

typedef struct ASTOperatorNode_s {
    ASTNodeType type;
    ASTOperatorType operatorType;
    ASTNode* leftExpression;
    ASTNode* rightExpression;
} ASTOperatorNode;

typedef struct ASTSymbolExpressionNode_s {
    ASTNodeType type;
    ASTSymbol* symbol;
} ASTSymbolExpressionNode;

typedef struct ASTDeclarationNode_s {
    ASTNodeType type;
    ASTSymbol* symbol;
    ASTNode* initializer;
} ASTDeclarationNode;

typedef struct ASTLambdaNode_s {
    ASTNodeType type;
    ASTNode* parameters;
    ASTNode* expression;
} ASTLambdaExpressionNode;

typedef struct ASTLiteralType_s {
    
} ASTLiteralType;

typedef struct ASTModuleNode_s {
    ASTNodeType type;
    uint32_t statementCount;
    ASTNode** statement;
} ASTModuleNode;

typedef struct ASTAssignmentNode_s {
    ASTNodeType type;
    ASTSymbol* lvalueSymbol;
    ASTNode* rvalueExpression;
} ASTAssignmentNode;

typedef struct ASTLiteralNode_s {
    ASTNodeType type;
    ASTLiteralType literalType;
} ASTLiteralNode;

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

void String_init(String* string, char* s) {

    string->length = s == 0 ? 0 : strlen(s);
    string->capacity = 0;
    string->data = s;
}

String* String_new(char* s) {

    String* string = (String*)malloc(sizeof(String));

    if(!string) return string;

    String_init(string, s);

    return string;
}

char* String_append(String* target, String* source) {

    int reallocate = 0;
    
    //TODO: There has to be a fast math way to calculate the
    //      next highest power of 2 from an arbitrary number
    while((target->length + source->length) > target->capacity) {

        target->capacity = target->capacity == 0
            ? 1
            : (2 * target->capacity);

        reallocate = 1;
    }

    if(reallocate) {

        target->data = (char*)realloc(target->data, target->capacity);

        if(!target->data) return "Failed to reallocate string buffer";
    }

    //TODO: error handle
    strncpy(&target->data[target->length], source->data, source->length);

    target->length = target->length + source->length;

    return 0;
}

char* String_appendChar(String* string, char c) {

    String temp;

    temp.capacity = 0;
    temp.length = 1;
    temp.data = &c;

    String_append(string, &temp);
}

void String_cleanUp(String* string) {

    if(string->capacity > 0) free(string->data);

    free(string);
}

void ASTNode_cleanUp(ASTNode* node) { ASTNodeMethodsFor[node->type].cleanUp(node); }
void ASTNode_print(ASTNode* node) { ASTNodeMethodsFor[node->type].print(node); }
void ASTNode_writeOut(FILE* out_file, ASTNode* node) { /* TODO: Generate C from AST */ }

char* ASTSymbol_tryParse(FILE* in_file, ASTSymbol** symbol); 
void ASTSymbol_cleanUp(ASTSymbol* symbol);

char* Operator_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    printf("Trying to parse an operator\n");

    fpos_t original_position, before_op_read_position;

    if(fgetpos(in_file, &original_position) != 0) return "Failed to get file position";

    skip_whitespace(in_file);

    ASTNode* left_expr;

    char* left_error = SymbolExpression_tryParse(in_file, &left_expr);

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
        printf("Invalid op '%c'\n", c);

        fsetpos(in_file, &before_op_read_position);

        *node = left_expr;

        return 0;
    }

    skip_whitespace(in_file);
    
    ASTNode* right_expr;

    char* right_error = SymbolExpression_tryParse(in_file, &right_expr);

    if(right_error != 0) {
    
        fsetpos(in_file, &original_position);

        ASTNode_cleanUp(left_expr);

        return right_error;
    }

    ASTOperatorNode* operator =
        (ASTOperatorNode*)malloc(sizeof(ASTOperatorNode));

    if(operator == 0) {

        fsetpos(in_file, &original_position);

        ASTNode_cleanUp(left_expr);
        ASTNode_cleanUp(right_expr);

        return "Failed to allocate memory for an operator node";
    }

    operator->type = Operator;
    operator->operatorType = op_type;
    operator->leftExpression = left_expr;
    operator->rightExpression = right_expr;

    *node = (ASTNode*)operator;

    return 0;
}

char* Parameter_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    printf("Trying to parse a parameter\n");

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
    
    ASTSymbol* symbol;

    skip_whitespace(in_file);

    char* error = ASTSymbol_tryParse(in_file, &symbol);

    if(error != 0) return error;

    ASTParameterNode* parameter_node =
        (ASTParameterNode*)malloc(sizeof(ASTParameterNode));

    if(parameter_node == 0) {
    
        fsetpos(in_file, &original_position);

        ASTSymbol_cleanUp(symbol);

        return "Unable to allocate space for new parameter declaration node";
    }

    parameter_node->type = Parameter;
    parameter_node->symbol = symbol;

    *node = (ASTNode*)parameter_node;

    return 0;
}

char* ASTParameterListNode_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    printf("Trying to parse a parameter list\n");

    fpos_t original_position;

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

        char* error = Parameter_tryParse(in_file, &parameter);

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

    ASTParameterListNode* parameterList =
        (ASTParameterListNode*)malloc(sizeof(ASTParameterListNode));

    if(parameterList == 0) {

        if(paramDeclarationsCount > 0) free(paramDeclarations);

        fsetpos(in_file, &original_position);

        return "Failed to allocate memory for parameter list";
    }

    parameterList->type = ParameterList;
    parameterList->count = paramDeclarationsCount;
    parameterList->parameters = paramDeclarations;
    
    *node = (ASTNode*)parameterList;

    return 0;
}


void ASTSymbol_cleanUp(ASTSymbol* symbol) {

    String_cleanUp(symbol->text);

    free(symbol);
}

char* ASTSymbol_tryParse(FILE* in_file, ASTSymbol** symbol) {

    //TEMP
    printf("Trying to parse a symbol\n");

    fpos_t original_position, previous_position;
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

            char* error = String_appendChar(symbol_text, c);

            if(error != 0) {
        
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

    *symbol = (ASTSymbol*)malloc(sizeof(ASTSymbol));

    if(!(*symbol)) {

        fsetpos(in_file, &original_position);

        String_cleanUp(symbol_text);

        return "Couldn't allocate memory for ast symbol";
    }

    (*symbol)->text = symbol_text;

    return 0;
}

//TODO: We really need to be building an actual symbol table here
char* SymbolExpression_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    printf("Trying to parse a symbol expression\n");

    ASTSymbol* symbol;
    fpos_t original_position;

    if(fgetpos(in_file, &original_position) != 0) return "Failed to get file position";

    char* symbol_error = ASTSymbol_tryParse(in_file, &symbol);

    if(symbol_error != 0) return symbol_error;

    ASTSymbolExpressionNode* symbol_node =
        (ASTSymbolExpressionNode*)malloc(sizeof(ASTSymbolExpressionNode));

    if(symbol_node == 0) {

        ASTSymbol_cleanUp(symbol);
    
        fsetpos(in_file, &original_position);

        return "Couldn't allocate memory for a symbol expression node";
    }

    symbol_node->type = Expression;
    symbol_node->expressionType = SymbolExpression;
    symbol_node->symbol = symbol;

    *node = (ASTNode*)symbol_node;

    return 0;
}

char* Declaration_tryParse(FILE* in_file, ASTNode** node); 

char* Lambda_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    printf("Trying to parse a lambda\n");

    ASTNode* parameterList;
    fpos_t original_position;

    if(fgetpos(in_file, &original_position) != 0) return "Failed to get file position";

    char* pl_error = ParameterList_tryParse(in_file, &parameterList);

    if(pl_error != 0) return pl_error;

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

    ASTLambdaExpressionNode* lambda =
        (ASTLambdaExpressionNode*)malloc(sizeof(ASTLambdaExpressionNode));

    if(!lambda)  {

        ASTNode_cleanUp(parameterList);
        ASTNode_cleanUp(expression);

        fsetpos(in_file, &original_position);

        return expression_error;
    }

    lambda->type = Lambda;
    lambda->parameters = parameterList;
    lambda->expression = expression;

    *node = (ASTNode*)lambda;

    return 0;
}

char* Expression_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    printf("Trying to parse an expression\n");

    char* error;
    
    if(Lambda_tryParse(in_file, node) == 0) return 0;
    if((error = Operator_tryParse(in_file, node)) == 0) return 0;

    return error;
}

char* Declaration_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    printf("Trying to parse a declaration\n");

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
    
    ASTSymbol* lvalue;

    skip_whitespace(in_file);

    char* error = ASTSymbol_tryParse(in_file, &lvalue);

    if(error != 0) return error;

    skip_whitespace(in_file);

    char eq;

    ASTNode* rvalue = 0;

    if(fread(&eq, 1, 1, in_file) == 1 && eq == '=') {

        error = Expression_tryParse(in_file, &rvalue);

        if(error != 0) {

            fsetpos(in_file, &original_position);
            ASTSymbol_cleanUp(lvalue);

            return error;
        }
    }

    skip_whitespace(in_file);

    char sc;

    if(fread(&sc, 1, 1, in_file) != 1 || sc != ';') {

        fsetpos(in_file, &original_position);
        ASTSymbol_cleanUp(lvalue);

        if(rvalue != 0) ASTNode_cleanUp(rvalue);

        return "No semicolon following logical end of declaration statement";
    }

    ASTDeclarationNode* declaration_node =
        (ASTDeclarationNode*)malloc(sizeof(ASTDeclarationNode))

    if(declaration_node == 0) {
    
        fsetpos(in_file, &original_position);
        ASTSymbol_cleanUp(lvalue);
        if(rvalue != 0) ASTNode_cleanUp(rvalue);

        return "Unable to allocate space for new declaration statement node";
    }

    declaration_node->type = Declaration;
    declaration_node->symbol = lvalue;
    declaration_node->initializer = rvalue;

    *node = (ASTNode*)declaration_node;

    return 0;
}

char* ASTStatementNode_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    printf("Trying to parse a statement\n");

    skip_whitespace(in_file);

    char* error;

    if((error = Declaration_tryParse(in_file, node)) == 0) return 0;

    return error;
}

char* Module_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    printf("Trying to parse a module\n");

    int statementCapacity = 0;
    char* inner_error = 0;
    ASTNode* new_statement;

    ASTModuleNode* module_node = (ASTModuleNode*)malloc(sizeof(ASTModuleNode));

    if(module_node == 0) return "Unable to allocate memory for a module node";

    module_node->type = Module;
    module_node->statementCount = 0;
    module_node->statement = 0;

    while((!feof(in_file)) && ((inner_error = ASTStatementNode_tryParse(in_file, &new_statement)) == 0)) {
        
        if(statementCapacity < (module_node->statementCount + 1)) {

            int nextCapacity = (statementCapacity == 0) ? 1 : (statementCapacity * 2);
        
            module_node->statement = (ASTStatementNode**)realloc(
                module_node->statement,
                nextCapacity * sizeof(ASTStatementNode*) );

            if(module_node->statement == 0) {

                inner_error = "Failed to allocate statement memory when constructing module node";

                break;
            }

            statementCapacity = nextCapacity;
        }

        module_node->statement[module_node->statementCount++] = new_statement;

        skip_whitespace(in_file);
    }

    *node = (ASTNode*)module_node;

    return inner_error;
}

int main(int argc, char* argv[]) {

    if(argc < 2) {

        printf("Usage: yc <in_file.y> [-o out_file | -t out_file.c]\n");

        return 0;
    }

    //Compile functions of the form ([args]) => expression; into a C function
    char* in_name = argv[1];

    FILE* in_file = fopen(in_name, "r");

    ASTNode* module_ast;

    char* error_message = Module_tryParse(in_file, &module_ast);

    if(error_message) {
        
        printf("Compilation failed: %s\n", error_message);

        ASTNode_cleanUp(module_ast); 

        return 1;
    }

    ASTNode_print(module_ast, 0);

    //TODO: Actually parse command line args as described
    FILE* out_file = fopen(argv[2], "w");

    ASTNode_writeOut(out_file, module_ast);

    fclose(out_file);

    ASTNode_cleanUp(module_ast);

    return 0;
}
