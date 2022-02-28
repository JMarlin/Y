#include <inttypes.h>
#include <stdio.h>

typedef enum {
    Module;
    Statement;
    Expression;
    SymbolReference;
    Lambda;
    LiteralValue;
    Operator;
} ASTNodeType;

typedef enum {
    SymbolDeclaration;
} ASTStatementType;

typedef enum {
    Assignment;
    Addition;
} ASTExpressionType;

typedef struct ASTNode_s {
    ASTNodeType type;
} ASTNode;

typedef struct ASTModuleNode_s {
    ASTNodeType type;
    uint32_t statementCount;
    ASTStatementNode* statement;
} ASTModuleNode;

typedef struct ASTStatementNode_s {
    ASTNodeType type;
    ASTStatementType statementType;
} ASTStatementNode;

typedef struct ASTExpressionNode_s {
    ASTNodeType type;
    ASTExpressionType expressionType;
} ASTExpressionNode;

typedef struct ASTSymbol_s {
    char* text;
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

typedef struct ASTUnt32LiteralExpression_s {
    ASTNodeType type;
    ASTExpressionType expressionType;
    ASTLiteralType literalType;
    uint32_t value;
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
