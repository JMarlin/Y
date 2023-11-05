#ifndef AST_H
#define AST_H

struct ASTNode_s;
struct ASTNodeMethods_s;
struct ASTSymbolNode_s;
struct ASTParameterNode_s;
struct ASTParameterListNode_s;
struct ASTOperatorNode_s;
struct ASTDeclarationNode_s;
struct ASTLambdaNode_s;
struct ASTModuleNode_s;
struct ASTAssignmentNode_s;

typedef void (*ASTNodePrinter)(struct ASTNode_s*, int);
typedef void (*ASTNodeCleaner)(struct ASTNode_s*);
typedef int (*ASTNodePredicate)(struct ASTNode_s*);
typedef char* (*ASTNodeVisitor)(struct ASTNode_s*, void*);

typedef enum {
    Module,
    Declaration,
    Parameter,
    ParameterList,
    Operator,
    Lambda,
    Symbol,
    Invocation,
    ArgumentList,
    StringLiteral,
    NumberLiteral,
    ASTNodeTypeCount
} ASTNodeType;

#include "string.h"
#include "template.h"
#include <stddef.h>
#include <stdio.h>
    
typedef struct ASTNode_s {
    ASTNodeType type;
    int childCount;
    struct ASTNode_s** children;
    int attributeCount;
    void** attributes;
} ASTNode;

typedef struct ASTNodeMethods_s {
    ASTNodePrinter print;
    ASTNodeCleaner cleanUp;
} ASTNodeMethods;

#define AN_METHODS_DECL(n) \
    void AST ## n ## Node_print(ASTNode*, int); \
    void AST ## n ## Node_cleanUp(ASTNode*);

AN_METHODS_DECL(Module);
AN_METHODS_DECL(Declaration);
AN_METHODS_DECL(Parameter);
AN_METHODS_DECL(ParameterList);
AN_METHODS_DECL(Operator);
AN_METHODS_DECL(Lambda);
AN_METHODS_DECL(Symbol);
AN_METHODS_DECL(Invocation);
AN_METHODS_DECL(ArgumentList);
AN_METHODS_DECL(StringLiteral);
AN_METHODS_DECL(NumberLiteral);

#define AN_METHODS_STRUCT(n) \
    (ASTNodeMethods){ \
        AST ## n ## Node_print, \
        AST ## n ## Node_cleanUp, \
    } 

extern const ASTNodeMethods ASTNodeMethodsFor[];

#define SN_TEXT attributes[0]

#define PN_SYMBOL children[0]

typedef enum {
    OpAdd,
    OpSubtract,
    OpMultiply,
    OpDivide,
    OpInvalid
} ASTOperatorType;

extern const char* OperatorString[];

#define ON_LEFT_EXPR children[0]
#define ON_RIGHT_EXPR children[1]
#define ON_OPERATOR attributes[0]

#define DN_SYMBOL children[0]
#define DN_INITIALIZER children[1]

#define LN_PARAMS children[0]
#define LN_EXPR children[1]

#define AN_SYMBOL children[0]
#define AN_EXPR children[1]

#define IN_SYMBOL children[0]
#define IN_ARGS children[1]

#define SLN_STRING attributes[0]

#define NLN_NUMBER attributes[0]

char* ASTNode_create(ASTNode** node, ASTNodeType type, int childCount, int attributeCount); 

void ASTNode_cleanUp(ASTNode* node); 

void ASTNode_print(ASTNode* node, int depth); 

char* ASTNode_forAll(ASTNode* root, ASTNodeVisitor visit, void* args); 

int ASTNode_IsLambda(ASTNode* node); 
int ASTNode_IsDeclaration(ASTNode* node);
int ASTNode_IsOperator(ASTNode* node);
int ASTNode_IsSymbol(ASTNode* node);
int ASTNode_IsStringLiteral(ASTNode* node);
int ASTNode_IsNumberLiteral(ASTNode* node);
int ASTNode_IsInvocation(ASTNode* node);

int ASTOperatorNode_OperatorIsAdd(ASTNode* node);

int ASTOperatorNode_OperatorIsSub(ASTNode* node);

int ASTOperatorNode_OperatorIsMul(ASTNode* node);

int ASTOperatorNode_OperatorIsDiv(ASTNode* node);

char* ASTNode_getChildByPath(ASTNode* in_node, String* path, String** rest_str,
    ASTNode** out_node); 

char* ASTNode_getAttributeByPath(ASTNode* node, String* path, void** attribute); 

char* ASTNode_renderTemplate(ASTNode* node, struct TemplateConfig_s* config, String** out_string); 
char* ASTNode_writeOut(FILE* out_file, struct TemplateConfig_s* config, ASTNode* node);

void ASTModuleNode_print(ASTNode* node, int depth);
void ASTModuleNode_cleanUp(ASTNode* node);

void ASTDeclarationNode_print(ASTNode* node, int depth);
void ASTDeclarationNode_cleanUp(ASTNode* node);

void ASTParameterNode_print(ASTNode* node, int depth);
void ASTParameterNode_cleanUp(ASTNode* node);

void ASTParameterListNode_print(ASTNode* node, int depth);
void ASTParameterListNode_cleanUp(ASTNode* node);

void ASTSymbolNode_print(ASTNode* node, int depth);
void ASTSymbolNode_cleanUp(ASTNode* node);

void ASTOperatorNode_print(ASTNode* node, int depth);
void ASTOperatorNode_cleanUp(ASTNode* node);

void ASTLambdaNode_print(ASTNode* node, int depth);
void ASTLambdaNode_cleanUp(ASTNode* node);

void ASTInvocationNode_print(ASTNode* node, int depth);
void ASTInvocationNode_cleanUp(ASTNode* node);

void ASTArgumentListNode_print(ASTNode* node, int depth);
void ASTArgumentListNode_cleanUp(ASTNode* node);

void ASTStringLiteralNode_print(ASTNode* node, int depth);
void ASTStringLiteralNode_cleanUp(ASTNode* node);

void ASTNumberLiteralNode_print(ASTNode* node, int depth);
void ASTNumberLiteralNode_cleanUp(ASTNode* node);

#endif
