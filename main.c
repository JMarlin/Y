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
AN_METHODS_DECL(Lambda);
AN_METHODS_DECL(LiteralValue);
AN_METHODS_DECL(Operator);
AN_METHODS_DECL(Parameter);
AN_METHODS_DECL(ParameterList);

#define AN_METHODS_STRUCT(n) \
    (ASTNodeMethods){ AST ## n ## Node_print, AST ## n ## Node_cleanUp }

typedef struct ASTNodeMethods_s {
    ASTNodePrinter print;
    ASTNodeCleaner cleanUp;
} ASTNodeMethods;

const ASTNodeMethods ASTNodeMethodsFor[] = {
    AN_METHODS_STRUCT(Module),
    AN_METHODS_STRUCT(Statement),
    AN_METHODS_STRUCT(Expression),
    AN_METHODS_STRUCT(Lambda),
    AN_METHODS_STRUCT(LiteralValue),
    AN_METHODS_STRUCT(Operator),
    AN_METHODS_STRUCT(Parameter),
    AN_METHODS_STRUCT(ParameterList)
};

typedef enum {
    SymbolDeclaration,
    Declaration
} ASTStatementType;

typedef enum {
    Assignment,
    Addition
} ASTExpressionType;

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
    ASTParameterNode* parameters;
} ASTParameterListNode;

typedef struct ASTExpressionNode_s {
    ASTNodeType type;
    ASTExpressionType expressionType;
} ASTExpressionNode;

typedef struct ASTDeclarationStatementNode_s {
    ASTNodeType type;
    ASTStatementType statementType;
    ASTSymbol* symbol;
    ASTExpressionNode* initializer;
} ASTDeclarationStatementNode;

typedef struct ASTLambdaExpressionNode_s {
    ASTNodeType type;
    ASTExpressionType expressionType;
    int parameterDeclarationCount;
    ASTDeclarationStatementNode* parameterDeclarations;
    ASTExpressionNode* expression;
} ASTLambdaExpressionNode;

typedef struct ASTLiteralType_s {
    
} ASTLiteralType;

typedef struct ASTModuleNode_s {
    ASTNodeType type;
    uint32_t statementCount;
    ASTStatementNode** statement;
} ASTModuleNode;

typedef struct ASTAssignmentExpression_s {
    ASTNodeType type;
    ASTExpressionType expressionType;
    ASTSymbol* lvalueSymbol;
    ASTExpressionNode* rvalueExpression;
} ASTAssignmentExpression;

typedef struct ASTAdditionExpression_s {
    ASTNodeType type;
    ASTExpressionType expressionType;
    ASTExpressionNode* leftAddend;
    ASTExpressionNode* rightAddend;
} ASTAdditionExpression;

typedef struct ASTLiteralExpression_s {
    ASTNodeType type;
    ASTExpressionType expressionType; 
    ASTLiteralType literalType;
} ASTLiteralExpression;

typedef struct ASTUInt32LiteralExpression_s {
    ASTNodeType type;
    ASTExpressionType expressionType;
    ASTLiteralType literalType;
    uint32_t value;
} ASTUInt32LiteralExpression;

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

void ParameterListNode_print(ASTNode* node ) {

    printf("ParameterListNode_print: Not implemented\n");
}

void ParameterListNode_cleanUp(ASTNode* node) {

    printf("ParameterListNode_cleanUp: Not implemented\n");
}

char* ASTSymbol_tryParse(FILE* in_file, ASTSymbol** symbol); 
void ASTSymbol_cleanUp(ASTSymbol* symbol);

char* ASTParameter_tryParse(FILE* in_file, ASTNode** node) {

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

        return "Parameter declaration did not begin with 'var' keyword";
    }
    
    ASTSymbol* symbol;

    skip_whitespace(in_file);

    char* error = ASTSymbol_tryParse(in_file, &symbol);

    if(error != 0) return error;

    ASTParameterNode** parameter_node =
        (ASTParameterNode**)node;

    if(((*parameter_node) =
        (ASTParameterNode*)malloc(sizeof(ASTParameterNode))) == 0) {
    
        fsetpos(in_file, &original_position);
        ASTSymbol_cleanUp(symbol);

        return "Unable to allocate space for new parameter declaration node";
    }

    (*parameter_node)->type = Parameter;
    (*parameter_node)->symbol = symbol;
}

char* ASTParameterListNode_tryParse(FILE* in_file, ASTNode** node) {

    fpos_t original_position;

    if(fgetpos(in_file, &original_position) != 0) return "Failed to get file position";

    skip_whitespace(in_file);

    char c;

    if(fread(&c, 1, 1, in_file) != 1) return "Unexpected EOF starting argument list";

    if(c != '(') return "Expected '(' at start of argument list";

    int paramDeclarationsCapacity = 0;
    int paramDeclarationsCount = 0;

    ASTParameterNode* paramDeclarations = 0;

    while(1) {
    
        ASTParameterNode* parameter;

        char* error = ASTParameter_tryParse(
            in_file,
            (ASTNode**)&parameter );

        if(error) {

            if(paramDeclarationsCount > 0) free(paramDeclarations);

            fsetpos(in_file, &original_position);

            return error;
        }

        if(paramDeclarationsCapacity == paramDeclarationsCount) {

            int new_size = paramDeclarationsCapacity == 0
                ? 1
                : (2 * paramDeclarationsCapacity);
    
            ASTParameterNode* newParametersPtr =
                (ASTParameterNode*)malloc(
                    sizeof(ASTParameterNode) * new_size );

            if(newParametersPtr != 0) {

                paramDeclarations = newParametersPtr;

                continue;
            }

            if(paramDeclarationsCount > 0) free(paramDeclarations);

            fsetpos(in_file, &original_position);

            return "Failed to allocate memory for parameter list";
        }

        paramDeclarations[paramDeclarationsCount++] = *parameter;

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
    
    (*node) = (ASTNode*)parameterList;

    return 0;
}


void ASTSymbol_cleanUp(ASTSymbol* symbol) {

    String_cleanUp(symbol->text);

    free(symbol);
}

char* ASTSymbol_tryParse(FILE* in_file, ASTSymbol** symbol) {

    fpos_t original_position;
    char c = 0;
    String* symbol_text = String_new(0);

    if(!symbol_text) return "Unable to allocate String for symbol text";

    if(fgetpos(in_file, &original_position) != 0) return "Failed to get file position";

    skip_whitespace(in_file);

    for(int i = 0; ; i++) {

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

        break;
    }

    (*symbol) = (ASTSymbol*)malloc(sizeof(ASTSymbol));

    if(!(*symbol)) {

        String_cleanUp(symbol_text);

        return "Couldn't allocate memory for ast symbol";
    }

    (*symbol)->text = symbol_text;

    return 0;
}

void ASTNode_cleanUp(ASTNode* node) { ASTNodeMethodsFor[node->type].cleanUp(node); }

void ASTNode_print(ASTNode* node) { ASTNodeMethodsFor[node->type].print(node); }

void ASTNode_writeOut(FILE* out_file, ASTNode* node) { /* TODO: Generate C from AST */ }

typedef char* (*ExpressionNodeParser)(FILE*, ASTExpressionNode**);

#define EN_METHODS_DECL(n) \
    char* n ## Expression_tryParse(FILE*, ASTExpressionNode**)

EN_METHODS_DECL(Operator);
EN_METHODS_DECL(Lambda);

typedef enum {
    Operator,
    Lambda,
    ExpressionTypeCount
} ExpressionType;

#define EN_METHODS_STRUCT(n) \
    (ExpressionNodeMethods){ n ## Expression_tryParse }

typedef struct ExpressionNodeMethods_s {
    ExpressionNodeParser tryParse;
} ExpressionNodeMethods;

const ExpressionNodeMethods ExpressionMethodsFor[] = {
    EN_METHODS_STRUCT(Lambda),
    EN_METHODS_STRUCT(Operator)
};

void ASTModuleNode_print(ASTNode* node) {
    printf("ASTModuleNode_print: Not implemented\n");
}

void ASTStatementNode_print(ASTNode* node) {
    printf("ASTStatementNode_print: Not implemented\n");
}

void ASTExpressionNode_print(ASTNode* node) {
    printf("ASTExpressionNode_print: Not implemented\n");
}

void ASTLambdaNode_print(ASTNode* node) {
    printf("ASTLambdaNode_print: Not implemented\n");
}

void ASTLiteralValueNode_print(ASTNode* node) {
    printf("ASTLiteralValueNode_print: Not implemented\n");
}

void ASTOperatorNode_print(ASTNode* node) {
    printf("ASTOperatorNode_print: Not implemented\n");
}

void ASTModuleNode_cleanUp(ASTNode* node) {
    printf("ASTModuleNode_cleanUp: Not implemented\n");
}

void ASTStatementNode_cleanUp(ASTNode* node) {
    printf("ASTStatementNode_cleanUp: Not implemented\n");
}

void ASTExpressionNode_cleanUp(ASTNode* node) {
    printf("ASTExpressionNode_cleanUp: Not implemented\n");
}

void ASTLambdaNode_cleanUp(ASTNode* node) {
    printf("ASTLambdaNode_cleanUp: Not implemented\n");
}

void ASTLiteralValueNode_cleanUp(ASTNode* node) {
    printf("ASTLiteralValueNode_cleanUp: Not implemented\n");
}

void ASTOperatorNode_cleanUp(ASTNode* node) {
    printf("ASTOperatorNode_cleanUp: Not implemented\n");
}

char* OperatorExpression_tryParse(FILE* in_file, ASTExpressionNode** expression_node) {
    return "OperatorExpression_tryParse: Not implemented";
}

void ASTDeclarationStatement_cleanUp(ASTStatementNode* node) {
    printf("ASTDeclarationStatement_cleanUp: Not implemented\n");
}

char* ASTExpressionNode_tryParse(FILE* in_file, ASTExpressionNode** expression_node); 
char* ASTDeclarationStatement_tryParse(FILE* in_file, ASTStatementNode** statement_node); 

char* LambdaExpression_tryParse(FILE* in_file, ASTExpressionNode** expression_node) {

    ASTParameterList* parameterList;
    
    char* pl_error = ASTParameterListNode_tryParse(in_file, (ASTNode*)&parameterList);

    if(pl_error != 0) return pl_error;

    skip_whitespace(in_file);

    char arrow_buf[2];

    if(fread(arrow_buf, 1, 2, in_file) != 2) {

        if(paramDeclarationsCount > 0) free(paramDeclarations);
        
        fsetpos(in_file, &original_position);

        return "Unexpected EOF after lambda expression argument list";
    }

    if(arrow_buf[0] != '=' || arrow_buf[1] != '>')  {

        if(paramDeclarationsCount > 0) free(paramDeclarations);

        fsetpos(in_file, &original_position);

        return "Expected '=>' following lambda argument list";
    }

    skip_whitespace(in_file);

    ASTExpressionNode* expression;

    char* expression_error = ASTExpressionNode_tryParse(in_file, &expression);

    if(expression_error != 0)  {

        if(paramDeclarationsCount > 0) free(paramDeclarations);

        fsetpos(in_file, &original_position);

        return expression_error;
    }

    ASTLambdaExpressionNode* lambda =
        (ASTLambdaExpressionNode*)malloc(sizeof(ASTLambdaExpressionNode));

    if(!lambda)  {

        if(paramDeclarationsCount > 0) free(paramDeclarations);

        ASTExpressionNode_cleanUp((ASTNode*)expression);

        fsetpos(in_file, &original_position);

        return expression_error;
    }

    lambda->type = Expression;
    lambda->expressionType = Lambda;
    lambda->parameterDeclarationCount = paramDeclarationsCount;
    lambda->parameterDeclarations = paramDeclarations;
    lambda->expression = expression;

    *expression_node = (ASTExpressionNode*)lambda;

    return 0;
}

char* ASTExpressionNode_tryParse(FILE* in_file, ASTExpressionNode** expression_node) {

    char *error, *first_error;
    
    for(int i = 0; i < ExpressionTypeCount; i++)
        if((error = ExpressionMethodsFor[i].tryParse(in_file, expression_node)) == 0)
            return 0;
        else if(i == 0)
            first_error = error;

    return first_error;
}

char* ASTDeclarationStatement_tryParse(FILE* in_file, ASTStatementNode** statement_node) {

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

    ASTExpressionNode* rvalue = 0;

    if(fread(&eq, 1, 1, in_file) == 1 && eq == '=') {

        error = ASTExpressionNode_tryParse(in_file, &rvalue);

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

        if(rvalue != 0) ASTNode_cleanUp((ASTNode*)rvalue);

        return "No semicolon following logical end of declaration statement";
    }

    ASTDeclarationStatementNode** declaration_statement_node =
        (ASTDeclarationStatementNode**)statement_node;

    if(((*declaration_statement_node) =
        (ASTDeclarationStatementNode*)malloc(sizeof(ASTDeclarationStatementNode))) == 0) {
    
        fsetpos(in_file, &original_position);
        ASTSymbol_cleanUp(lvalue);
        if(rvalue != 0) ASTNode_cleanUp((ASTNode*)rvalue);

        return "Unable to allocate space for new declaration statement node";
    }

    (*declaration_statement_node)->type = Statement;
    (*declaration_statement_node)->statementType = Declaration;
    (*declaration_statement_node)->symbol = lvalue;
    (*declaration_statement_node)->initializer = rvalue;
}

char* ASTStatementNode_tryParse(FILE* in_file, ASTStatementNode** statement_node) {

    *statement_node = (ASTStatementNode*)malloc(sizeof(ASTStatementNode));
    (*statement_node)->type = Statement;

    if(!(*statement_node)) {
        return "Failed to allocate memory for a statement node";
    }

    skip_whitespace(in_file);

    char* first_error;

    if((first_error = ASTDeclarationStatement_tryParse(in_file, statement_node)) == 0) return 0;

    return first_error;
}

char* ASTModuleNode_tryParse(FILE* in_file, ASTModuleNode** module_node) {

    int statementCapacity = 0;
    char* inner_error = 0;
    ASTStatementNode* new_statement;

    (*module_node) = (ASTModuleNode*)malloc(sizeof(ASTModuleNode));

    if((*module_node) == 0) return "Unable to allocate memory for a module node";

    (*module_node)->type = Module;
    (*module_node)->statementCount = 0;
    (*module_node)->statement = 0;

    while((!feof(in_file)) && ((inner_error = ASTStatementNode_tryParse(in_file, &new_statement)) == 0)) {
        
        if(statementCapacity < ((*module_node)->statementCount + 1)) {

            int nextCapacity = (statementCapacity == 0) ? 1 : (statementCapacity * 2);
        
            (*module_node)->statement = (ASTStatementNode**)realloc(
                (*module_node)->statement,
                nextCapacity * sizeof(ASTStatementNode*) );

            if(!(*module_node)->statement) {

                inner_error = "Failed to allocate statement memory when constructing module node";

                break;
            }

            statementCapacity = nextCapacity;
        }

        (*module_node)->statement[(*module_node)->statementCount++] = new_statement;

        skip_whitespace(in_file);
    }

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

    ASTModuleNode* module_ast;

    char* error_message = ASTModuleNode_tryParse(in_file, &module_ast);

    if(error_message) {
        
        printf("Compilation failed: %s\n", error_message);

        ASTModuleNode_cleanUp((ASTNode*)module_ast); 

        return 1;
    }

    ASTNode_print((ASTNode*)module_ast);

    //TODO: Actually parse command line args as described
    FILE* out_file = fopen(argv[2], "w");

    ASTNode_writeOut(out_file, (ASTNode*)module_ast);

    fclose(out_file);

    ASTNode_cleanUp((ASTNode*)module_ast);

    return 0;
}
