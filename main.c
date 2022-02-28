#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    Module,
    Statement,
    Expression,
    SymbolReference,
    Lambda,
    LiteralValue,
    Operator 
} ASTNodeType;

typedef enum {
    SymbolDeclaration
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

typedef struct ASTNode_s {
    ASTNodeType type;
} ASTNode;

typedef struct ASTStatementNode_s {
    ASTNodeType type;
    ASTStatementType statementType;
} ASTStatementNode;

typedef struct ASTLiteralType_s {
    
} ASTLiteralType;

typedef struct ASTModuleNode_s {
    ASTNodeType type;
    uint32_t statementCount;
    ASTStatementNode* statement;
} ASTModuleNode;

typedef struct ASTExpressionNode_s {
    ASTNodeType type;
    ASTExpressionType expressionType;
} ASTExpressionNode;

typedef struct ASTSymbol_s {
    String* text;
} ASTSymbol;

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

    while(1) {

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

char* ASTExpressionNode_tryParse(FILE* in_file, ASTExpressionNode** expression_node) {

    //TODO: Here
}

char* ASTDeclarationStatement_tryParse(FILE* in_file, ASTStatementNode** statement_node) {

    fpos_t original_position;

    if(fgetpos(in_file, &original_position) != 0) return "Failed to get file position";

    skip_whitespace(in_file);

    char kw_buf[4];
    const char* var_keyword = "var";

    if(fread(kw_buf, 1, 4, in_file) != 4) return "Unexpected EOF";

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

    if(fread(&eq, 1, 1, in_file) != 1 || eq != '=') {

        fsetpos(in_file, &original_position);
        ASTSymbol_cleanUp(lvalue);

        return "Unexpected EOF";
    }

    ASTExpressionNode* rvalue;

    error = ASTExpressionNode_tryParse(in_file, &rvalue);

    if(error != 0) {

        fsetpos(in_file, &original_position);
        ASTSymbol_cleanUp(lvalue);

        return error;
    }

    skip_whitespace(in_file);

    char sc;

    if(fread(&sc, 1, 1, in_file) != 1 || sc != ';') {

        fsetpos(in_file, &original_position);
        ASTSymbol_cleanUp(lvalue);
        ASTSymbol_cleanUp(rvalue);

        return "No semicolon following logical end of declaration statement";
    }

    ASTDeclarationStatementNode** declaration_statement_node = (ASTDeclarationStatementNode**)statement_node;

    if((*declaration_statement_node) = (ASTDeclarationStatementNode*)malloc(sizeof(ASTDeclarationStatementNode))) {
    
        fsetpos(in_file, &original_position);
        ASTSymbol_cleanUp(lvalue);
        ASTSymbol_cleanUp(rvalue);

        return "Unable to allocate space for new declaration statement node";
    }

    (*declaration_statement_node)->type = ASTNodeType.Statement;
    (*declaration_statement_node)->statementType = ASTStatementType.Declaration;
    (*declaration_statement_node)->lvalueSymbol = lvalue;
    (*declaration_statement_node)->rvalueExpression = rvalue;
}

char* ASTStatementNode_tryParse(FILE* in_file, ASTStatementNode** statement_node) {

    *statement_node = (ASTStatementNode*)malloc(sizeof(ASTStatementNode));
    (*statement_node)->type = ASTNodeType.Statement;

    if(!(*statement_node)) {
        return "Failed to allocate memory for a statement node";
    }

    if(ASTDeclarationStatement_tryParse(in_file, statement_node) == 0) return 0;

    return "Expected a statement but did not find one";
}

char* ASTModuleNode_tryParse(FILE* in_file, ASTModuleNode* module_node) {

    int statementCapacity = 0;
    char* inner_error = 0;
    ASTStatementNode* new_statement;

    module_node->type = ASTNodeType.Module;
    module_node->statementCount = 0;
    module_node->statement = 0;

    while(not at end of file && ((inner_error = ASTStatementNode_tryParse(in_file, &new_statement)) == 0)) {
        
        if(statementCapacity < (module_node->statementCount + 1)) {

            int nextCapacity = (statementCapacity == 0) ? 1 : (statementCapacity * 2);
        
            module_node->statement = (ASTStatementNode*)realloc(
                module_node->statement,
                nextCapacity * sizeof(ASTStatementNode) );

            if(!module_node->statement) {

                inner_error = "Failed to allocate statement memory when constructing module node";

                break;
            }

            statementCapacity = nextCapacity;
        }

        module_node->statement[module_node->statementCount++] = new_statement;
    }

    return inner_error;
}

int main(int argc, char* argv[]) {

    if(argc < 1) {

        printf("Usage: yc <in_file.y> [-o out_file | -t out_file.c]");

        return 0;
    }

    //Compile functions of the form ([args]) => expression; into a C function
    char* in_name = argv[0];

    FILE* in_file = fopen(in_name, "r");

    ASTModuleNode module_ast;

    char* error_message = ASTModuleNode_tryParse(in_file, &module_ast);

    if(error_message) {
        
        printf("Compilation failed: %s\n", warning_message);

        ASTModuleNode_cleanUp(module_ast);

        return 1;
    }

    ASTModuleNode_debugPrint(&module_ast);

    //TODO: Actually parse command line args as described
    FILE* out_file = argv[1];

    ASTModuleNode_writeOut(out_file, module_ast);

    fclose(out_file);

    ASTModuleNode_cleanUp(module_ast);

    return 0;
}
