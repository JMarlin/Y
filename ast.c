#include "ast.h"
#include "helpers.h"
#include <stdlib.h>
#include <string.h>

const char* OperatorString[] = {
    "Add",
    "Subtract",
    "Multiply",
    "Divide",
    "INVALID"
};

const ASTNodeMethods ASTNodeMethodsFor[] = {
    AN_METHODS_STRUCT(Module),
    AN_METHODS_STRUCT(Declaration),
    AN_METHODS_STRUCT(Parameter),
    AN_METHODS_STRUCT(ParameterList),
    AN_METHODS_STRUCT(Operator),
    AN_METHODS_STRUCT(Lambda),
    AN_METHODS_STRUCT(Symbol),
    AN_METHODS_STRUCT(Invocation),
    AN_METHODS_STRUCT(ArgumentList),
    AN_METHODS_STRUCT(StringLiteral),
    AN_METHODS_STRUCT(NumberLiteral)
};

char* ASTNode_create(ASTNode** node, ASTNodeType type, int childCount, int attributeCount) {

    *node = (ASTNode*)malloc(sizeof(ASTNode));

    if(*node == 0) {
        
        return "Could not allocate space for an AST node";
    }

    if(childCount == 0) {
    
        (*node)->children = 0;
    } else {

        (*node)->children = (ASTNode**)malloc(sizeof(ASTNode*) * childCount);

        if((*node)->children == 0) {

            free(*node);

            return "Could not allocate space for AST node children";
        }
    }

    if(attributeCount == 0) {
    
        (*node)->attributes = 0;
    } else {

        (*node)->attributes = (void**)malloc(sizeof(void*) * attributeCount);

        if((*node)->attributes == 0) {

            free((*node)->attributes);
            free(*node);

            return "Could not allocate space for AST node children";
        }
    }

    (*node)->childCount = childCount;
    (*node)->attributeCount = attributeCount;
    (*node)->type = type;

    return 0;
}

void ASTNode_cleanUp(ASTNode* node) {
    
    for(int i = 0; i < node->childCount; i++) {

        ASTNode_cleanUp(node->children[i]);
    }
    
    ASTNodeMethodsFor[node->type].cleanUp(node);

    free(node);
}

void ASTNode_print(ASTNode* node, int depth) { ASTNodeMethodsFor[node->type].print(node, depth); }

typedef char* (*ASTNodeVisitor)(ASTNode*, void*);

char* ASTNode_forAll(ASTNode* root, ASTNodeVisitor visit, void* args) {

    char* error;

    if((error = visit(root, args)) != 0) return error;

    for(int i = 0; i < root->childCount; i++) {

        if((error = ASTNode_forAll(root->children[i], visit, args)) != 0) {

            return error;
        }
    }

    return 0;
}

int ASTNode_IsLambda(ASTNode* node) {
    
    return node->type == Lambda;
}

int ASTNode_IsDeclaration(ASTNode* node) {
    
    return node->type == Declaration;
}

int ASTNode_IsOperator(ASTNode* node) {

    return node->type == Operator;
}

int ASTNode_IsSymbol(ASTNode* node) {

    return node->type == Symbol;
}

int ASTNode_IsStringLiteral(ASTNode* node) {

    return node->type == StringLiteral;
}

int ASTNode_IsNumberLiteral(ASTNode* node) {

    return node->type == NumberLiteral;
}

int ASTNode_IsInvocation(ASTNode* node) {

    return node->type == Invocation;
}

int ASTOperatorNode_OperatorIsAdd(ASTNode* node) {

    return node->ON_OPERATOR == (void*)OpAdd;
}

int ASTOperatorNode_OperatorIsSub(ASTNode* node) {

    return node->ON_OPERATOR == (void*)OpSubtract;
}

int ASTOperatorNode_OperatorIsDiv(ASTNode* node) {

    return node->ON_OPERATOR == (void*)OpDivide;
}

int ASTOperatorNode_OperatorIsMul(ASTNode* node) {

    return node->ON_OPERATOR == (void*)OpMultiply;
}

char* ASTNode_getChildByPath(ASTNode* in_node, String* path, String** rest_str,
    ASTNode** out_node) {

    char* number_str;
    char* error;
    int i, j, child_idx;

    *out_node = in_node;

    if(path->length == 0) return 0;

    for(i = 0; i < path->length;) {

        if(path->data[i] != 'c') {

            if(path->data[i] == 'a' && rest_str != 0) break;  

            return 0;
        }

        if(++i == path->length) break;

        for(j = i; path->data[j] >= '0' && path->data[j] <= '9' && j < path->length; j++);

        if(j == i) return "Expected index number following 'c' in node path segment";

        number_str = strndup(&path->data[i], j - i);
        child_idx = strtol(number_str, 0, 0);
        free(number_str);

        if(child_idx >= (*out_node)->childCount) {

            return "Specified index in child path segment outside of child count";
        }

        *out_node = (*out_node)->children[child_idx];

        i = j;
    }

    if(rest_str == 0) return 0;

    if((error = String_sliceCString(&path->data[i], &path->data[path->length], rest_str)) != 0) {

        return error;
    }

    return 0;
}

char* ASTNode_getAttributeByPath(ASTNode* node, String* path, void** attribute) {

    char* error;
    int attr_idx;
    ASTNode* attr_node;
    char* clone_str;
    String* attr_path;

    if((error = ASTNode_getChildByPath(node, path, &attr_path, &attr_node)) != 0) return error;

    if(attr_path->length < 2) return "No attribute path at the end of node path";

    if(attr_path->data[0] != 'a') return "Expected 'a' at beginning of attribute path";

    clone_str = strndup(attr_path->data, attr_path->length);

    String_cleanUp(attr_path);

    if(clone_str == 0) return "Failed to allocate space for attribute index string";

    attr_idx = strtol(clone_str, 0, 0);

    free(clone_str);

    if(attr_idx >= attr_node->attributeCount) {

        return "Attribute index beyond range of node attributes";
    }

    *attribute = attr_node->attributes[attr_idx];

    return 0;
}

char* ASTNode_renderTemplate(ASTNode* node, TemplateConfig* config, String** out_string) {

    char* error;
    Template* template;
    String* template_name;

    if((template_name = String_new(config->baseTemplateName[node->type])) == 0) {
        
        return "Unable to allocate string for template name lookup";
    }

    error = Template_getCompiled(config, template_name, &template);

    String_cleanUp(template_name);

    if(error != 0) return error;

    if((error = Template_renderCompiled(template, node, out_string)) != 0) return error;

    return 0;
}

char* ASTNode_writeOut(FILE* out_file, TemplateConfig* config, ASTNode* node) {

    String* code_str;

    char* error = ASTNode_renderTemplate(node, config, &code_str);

    if(error != 0) return error;
 
    fprintf(out_file, "%.*s", code_str->length, code_str->data);
    String_cleanUp(code_str);

    return 0;
}

void ASTModuleNode_print(ASTNode* node, int depth) {

    print_indent(depth); printf("- Module\n");

    for(int i = 0; i < node->childCount; i++) {
        
        ASTNode_print(node->children[i], depth + 1);
    }
}

void ASTModuleNode_cleanUp(ASTNode* node) { }

void ASTDeclarationNode_print(ASTNode* node, int depth) {
    
    print_indent(depth); printf("- Declaration\n");
    print_indent(depth); printf("  Symbol:\n");

    ASTNode_print(node->DN_SYMBOL, depth + 1);

    print_indent(depth); printf("  Initializer:\n");

    ASTNode_print(node->DN_INITIALIZER, depth + 1);
}

void ASTDeclarationNode_cleanUp(ASTNode* node) { }

void ASTParameterNode_print(ASTNode* node, int depth) {

    print_indent(depth); printf("- Parameter\n");
    print_indent(depth); printf("  Symbol:\n");
    
    ASTNode_print(node->PN_SYMBOL, depth + 1);
}

void ASTParameterNode_cleanUp(ASTNode* node) { }

void ASTParameterListNode_print(ASTNode* node, int depth) {

    print_indent(depth); printf("- Parameter List\n");

    for(int i = 0; i < node->childCount; i++) {

        ASTNode_print(node->children[i], depth + 1);
    }
}

void ASTParameterListNode_cleanUp(ASTNode* node) { }

void ASTSymbolNode_print(ASTNode* node, int depth) {

    String* text = (String*)node->SN_TEXT;

    print_indent(depth); printf("- Symbol \n");
    print_indent(depth); printf("  Text: %.*s\n", text->length, text->data);
}

void ASTSymbolNode_cleanUp(ASTNode* node) {

    String_cleanUp((String*)node->SN_TEXT);
}

void ASTStringLiteralNode_print(ASTNode* node, int depth) {

    String* text = (String*)node->SLN_STRING;

    print_indent(depth); printf("- String literal: %.*s\n", text->length, text->data);
}

void ASTStringLiteralNode_cleanUp(ASTNode* node) {

    String_cleanUp((String*)node->SN_TEXT);
}

void ASTNumberLiteralNode_print(ASTNode* node, int depth) {

    long int value = (long int)node->NLN_NUMBER;

    print_indent(depth); printf("- Number literal: %ld\n", value);
}

void ASTNumberLiteralNode_cleanUp(ASTNode* node) { }

void ASTOperatorNode_print(ASTNode* node, int depth) {

    print_indent(depth); printf("- Operator\n");
    print_indent(depth); printf("  Operation: %s\n", OperatorString[(size_t)node->ON_OPERATOR]);
    print_indent(depth); printf("  LeftExpr:\n");
    ASTNode_print(node->ON_LEFT_EXPR, depth + 1);
    print_indent(depth); printf("  RightExpr:\n");
    ASTNode_print(node->ON_RIGHT_EXPR, depth + 1);
}

void ASTOperatorNode_cleanUp(ASTNode* node) { }

void ASTLambdaNode_print(ASTNode* node, int depth) {

    print_indent(depth); printf("- Lambda\n");
    print_indent(depth); printf("  Parameters:\n");

    ASTNode_print(node->LN_PARAMS, depth + 1);

    print_indent(depth); printf("  Body:\n");

    ASTNode_print(node->LN_EXPR, depth + 1);
}

void ASTLambdaNode_cleanUp(ASTNode* node) { }

void ASTInvocationNode_print(ASTNode* node, int depth) {

    print_indent(depth); printf("- Invocation:\n");
    print_indent(depth); printf("  Symbol:\n");

    ASTNode_print(node->IN_SYMBOL, depth + 1);

    print_indent(depth); printf("  Arguments:\n");

    ASTNode_print(node->IN_ARGS, depth + 1);
}

void ASTInvocationNode_cleanUp(ASTNode* node) { }

void ASTArgumentListNode_print(ASTNode* node, int depth) {

    print_indent(depth); printf("- Argument List\n");

    for(int i = 0; i < node->childCount; i++) {

        ASTNode_print(node->children[i], depth + 1);
    }
}

void ASTArgumentListNode_cleanUp(ASTNode* node) { }

