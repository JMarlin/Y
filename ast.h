#ifndef AST_H
#define AST_H

typedef enum {
    Module,
    Declaration,
    Parameter,
    ParameterList,
    Operator,
    Lambda,
    Symbol,
    ASTNodeTypeCount
} ASTNodeType;

#define AN_COMMON_FIELDS \
    ASTNodeType type; \
    int childCount; \
    struct ASTNode_s** children; \
    int attributeCount; \
    void** attributes;

typedef struct ASTNode_s {
    AN_COMMON_FIELDS;
} ASTNode;

char* ASTNode_create(
    ASTNode** node, ASTNodeType type, int childCount,
    int attributeCount, size_t nodeSize);

typedef void (*ASTNodePrinter)(ASTNode*, int);
typedef void (*ASTNodeCleaner)(ASTNode*);
typedef int (*ASTNodePredicate)(ASTNode*);

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

#define AN_METHODS_STRUCT(n) \
    (ASTNodeMethods){ \
        AST ## n ## Node_print, \
        AST ## n ## Node_cleanUp, \
    } 

const ASTNodeMethods ASTNodeMethodsFor[] = {
    AN_METHODS_STRUCT(Module),
    AN_METHODS_STRUCT(Declaration),
    AN_METHODS_STRUCT(Parameter),
    AN_METHODS_STRUCT(Lambda),
    AN_METHODS_STRUCT(Symbol)
};

#define SN_TEXT attributes[0]

typedef struct ASTSymbolNode_s {
    AN_COMMON_FIELDS;
} ASTSymbolNode;

#define PN_SYMBOL children[0]

typedef struct ASTParameterNode_s {
    AN_COMMON_FIELDS;
} ASTParameterNode;

typedef struct ASTParameterListNode_s {
    AN_COMMON_FIELDS;
} ASTParameterListNode;

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

typedef struct ASTOperatorNode_s {
    AN_COMMON_FIELDS;
    ASTOperatorType operatorType;
} ASTOperatorNode;

#define DN_SYMBOL children[0]
#define DN_INITIALIZER children[1]

typedef struct ASTDeclarationNode_s {
    AN_COMMON_FIELDS;
} ASTDeclarationNode;

#define LN_PARAMS children[0]
#define LN_EXPR children[1]

typedef struct ASTLambdaNode_s {
    AN_COMMON_FIELDS;
} ASTLambdaNode;

typedef struct ASTModuleNode_s {
    AN_COMMON_FIELDS;
} ASTModuleNode;

#define AN_SYMBOL children[0]
#define AN_EXPR children[1]

typedef struct ASTAssignmentNode_s {
    AN_COMMON_FIELDS;
} ASTAssignmentNode;


#endif
