#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    ASTNode** node, ASTNodeType type, int childCount, int attributeCount, size_t nodeSize) {

    *node = (ASTNode*)malloc(nodeSize);

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

typedef struct VoidList_s {
    int capacity;
    int count;
    void** data;
} VoidList;

void VoidList_init(VoidList* vlist) {
    vlist->capacity = 0;
    vlist->count = 0;
    vlist->data = 0;
}

char* VoidList_add(VoidList* vlist, void* entry) {

    //TODO: There has to be a fast math way to calculate the
    //      next highest power of 2 from an arbitrary number
    if(vlist->capacity < (vlist->count + 1)) {

        vlist->capacity = vlist->capacity == 0
            ? 1
            : (2 * vlist->capacity);

        vlist->data = (void**)realloc(vlist->data, vlist->capacity * sizeof(void*));

        if(!vlist->data) return "Failed to allocate space for a list";
    }
    
    vlist->data[vlist->count++] = entry;

    return 0;
}

void VoidList_cleanUp(VoidList* vlist) {

    if(vlist->capacity > 0) free(vlist->data);
}

void print_indent(int depth) {
    
    for(int i = 0; i < depth; i++) {
    
        printf("    ");
    }
}

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
    AN_METHODS_STRUCT(ParameterList),
    AN_METHODS_STRUCT(Operator),
    AN_METHODS_STRUCT(Lambda),
    AN_METHODS_STRUCT(Symbol)
};

typedef struct String_s {
    uint32_t capacity;
    uint32_t length;
    char* data;
} String;

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
    int original_capacity = target->capacity;
    
    //TODO: There has to be a fast math way to calculate the
    //      next highest power of 2 from an arbitrary number
    while((target->length + source->length) > target->capacity) {

        target->capacity = target->capacity == 0
            ? 1
            : (2 * target->capacity);

        reallocate = 1;
    }

    if(reallocate) {

        char* last_data = target->data;
        int precopy = original_capacity == 0 && target->data != 0;

        if(original_capacity == 0) target->data = 0;

        target->data = (char*)realloc(target->data, target->capacity);

        if(!target->data) return "Failed to reallocate string buffer";
     
        //TODO: error handle
        if(precopy) strncpy(target->data, last_data, target->length);
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

char* String_appendCString(String* string, char* s) {

    String temp;

    temp.capacity = 0;
    temp.length = strlen(s);
    temp.data = s;

    String_append(string, &temp);
}

char* String_sliceCString(char* start, char* end, String** string) {

    *string = (String*)malloc(sizeof(String));

    if(*string == 0) return "Failed to allocate space for a new sliced string";

    (*string)->capacity = 0;
    (*string)->length = (size_t)end - (size_t)start;
    (*string)->data = start;

    return 0;
}

void String_cleanUp(String* string) {

    if(string->capacity > 0) free(string->data);

    free(string);
}

typedef struct ASTSymbolNode_s {
    AN_COMMON_FIELDS;
    String* text;
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

const char* OperatorString[] = {
    "Add",
    "Subtract",
    "Multiply",
    "Divide",
    "INVALID"
};

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

void ASTNode_cleanUp(ASTNode* node) {
    
    for(int i = 0; i < node->childCount; i++) {

        ASTNode_cleanUp(node->children[i]);
    }
    
    ASTNodeMethodsFor[node->type].cleanUp(node);

    free(node);
}

void ASTNode_print(ASTNode* node, int depth) { ASTNodeMethodsFor[node->type].print(node, depth); }

char* ASTTree_findAllInner(ASTNode* root, ASTNodePredicate matches, VoidList* vlist) {

    char* error = 0;

    if(
        matches(root) &&
        ((error = VoidList_add(vlist, root)) != 0) ) {
    
        return error;
    }

    for(int i = 0; i < root->childCount; i++) {

        if((error = ASTTree_findAllInner(root->children[i], matches, vlist)) != 0) {

            return error;
        }
    }

    if(error != 0) {

        return error;
    }

    return 0;
}

char* ASTTree_findAll(ASTNode* root, ASTNodePredicate matches, VoidList* vlist) {

    VoidList_init(vlist);

    char* error = ASTTree_findAllInner(root, matches, vlist);

    if(error != 0) VoidList_cleanUp(vlist);

    return error;
}

int ASTNode_IsLambda(ASTNode* node) {
    
    return node->type == Lambda;
}

char* Lambda_generateCBody(ASTNode* node, int lambda_id, String** out_str) {

    char* error = 0;

    *out_str = String_new("int Lambda");

    if(*out_str == 0) {

        return "Unable to allocate a string for lambda body write-out";
    }

    char num_buf[50] = {0};

    sprintf(num_buf, "%i", lambda_id);

    if((error = String_appendCString(*out_str, num_buf)) != 0) {

        String_cleanUp(*out_str);

        return error;
    }

    if((error = String_appendCString(*out_str, "(")) != 0) {

        String_cleanUp(*out_str);

        return error;
    }

    for(int i = 0; i < node->LN_PARAMS->childCount; i++) {

        char* param_str = i == 0 ? "int " : ", int ";

        if((error = String_appendCString(*out_str, param_str)) != 0) {

            String_cleanUp(*out_str);

            return error;
        }   

        ASTSymbolNode* symbol = (ASTSymbolNode*)node->LN_PARAMS->children[i]->PN_SYMBOL;

        if((error = String_append(*out_str, symbol->text)) != 0) {

            String_cleanUp(*out_str);

            return error;
        }
    }

    if((error = String_appendCString(*out_str, ") {\n    return ")) != 0) {

        String_cleanUp(*out_str);

        return error;
    }

    return 0;
}

typedef struct Template_s {
    VoidList segments;
    VoidList expressions;
} Template;

typedef struct TemplateInfo_s {
    char* templateName;
    char* template;
    Template* compiledTemplate;
} TemplateInfo;

typedef struct TemplateConfig_s {
    char* baseTemplateName[ASTNodeTypeCount];
    int templateCount;
    TemplateInfo templateList[];
} TemplateConfig;

TemplateConfig CTemplateConfig = {
    {
    "", //Module
    "", //Declaration
    "", //Parameter
    "", //ParameterList
    "", //Operator
    "lambda_declarations", //Lambda
    "" //Symbol
    },
    3, 
    {
        {
            "lambda_declarations",
            "typedef int (*Lambda{{ia0}}Type){{tc0`param_type_list`}};\n", 0
        },
        {
            "param_type_list",
            "({{e`{{t`param_type`}}`}})", 0
        },
        {
            "param_type",
            "{{c`!first`, `}}int", 0
        }
    }
};

char* TemplateConfig_lookUp(TemplateConfig* config, String* template_name, TemplateInfo** template_info) {

    char* error;

    for(int i = 0; i < config->templateCount; i++) {

        if(
            strlen(config->templateList[i].templateName)
                == template_name->length &&
            (strncmp(
                config->templateList[i].templateName,
                template_name->data,
                template_name->length) == 0)) {

            *template_info = &config->templateList[i];
            
            return 0;
        }
    }

    return "Specified template name was not found in the template list";
}

typedef struct TemplateExpression_s {
    char typeCode;
    String* sourcePath;
    Template* template;
} TemplateExpression;

char* cstr_skip_whitespace(char** s, char* end, int expect_more) {

    for(; (*s) != end; (*s)++) if(**s > 0x20) return 0;

    return expect_more ? "Hit the end of a cstr while skipping whitespace" : 0;
}

char* Template_compile(TemplateConfig* config,  char** template_strp, Template** template);
char* Template_getCompiled(TemplateConfig* config, String* template_name, Template** out_template);

char* TemplateExpression_tryParse(
    TemplateConfig* config, char** sp, char* end_pos, TemplateExpression** out_expr) {

    char* s = *sp;
    char* error;
    TemplateExpression expr = {0};

    if((error = cstr_skip_whitespace(&s, end_pos, 1)) != 0) return error;

    if(s == end_pos) return "Expected a qualifier following expression type, but found end of string";

    expr.typeCode = *(s++);

    if(
        expr.typeCode != 'e' &&
        expr.typeCode != 'c' &&
        expr.typeCode != 't' &&
        expr.typeCode != 'i' &&
        expr.typeCode != 's'
    ) {

        return "Encountered an unrecognized template expression type code";
    }

    //Expand format:
    //e<child_path>`text_expr`
    //    child_path: rule for navigating to the child to expand
    //                if there is no path, the target is the current node
    //                if the path ends in a 's', the inner template is expanded only for the one child
    //                if the path does not end in an 's', the inner template is expanded for each
    //                child in the terminal node
    //    text_expr:  embedded template that will be compiled and inserted into the compiled expression
    if(expr.typeCode == 'e') {
        
        int len = 0;
       
        for(len = 0; &s[len] != end_pos; len++) if(s[len] == '`') break;

        if(&s[len] == end_pos) {

            //TODO: Clean up everything
            return "Hit end of expression looking for closing '`' following 'e' expression code";
        }

        if((error = String_sliceCString(s, &s[len], &expr.sourcePath)) != 0) {
    
            //TODO: Clean up everything
            return error;
        }

        s = &s[len + 1];

        if(s == end_pos) {

            //TODO: Clean up everything
            return "Hit end of expression beginning template body of 'e' expression";
        }

        if((error = Template_compile(config, &s, &expr.template)) != 0) {

            //TODO: Clean up everything
            return error;
        }

        //TODO: This doesn't make any sene, we have to get the number of characters advanced
        //      by Template_compile and only THEN can we proceed to check what this char is 
        if(*s != '`') {

            //TODO: Clean up everything
            return "Expected closing '`' following 'e' template expression body";
        }

        s++;
    }

    //Conditional format:
    //c`first | !first | <attribute_path> | !<attribute_path>`text_expr`
    //Conditions:
    //    first - true when this is either a standalone node or the first in an iteration
    //    !first - true when this is not the first node in an iteration
    //    attribute_path - the attribute is navigated to and assumed to be a boolean value
    //    !attribute_path - same as above, but inverts the value of the attribute
    //Value:
    //    text_expr: an embedded template which will be recursively compiled and placed in
    //               the expression's template field
    if(expr.typeCode == 'c') {

        int len = 0;
        
        if(*s != '`') {

            //TODO: Clean up everything
            return "Expected a '`' following 'c' template expression code";
        }

        for(len = 0; &s[len] != end_pos; len++) if(s[len] == '`') break;

        if(&s[len] == end_pos) {

            //TODO: Clean up everything
            return "Hit end of expression looking for closing '`' following 'c' expression code";
        }

        s = &s[len + 1];

        if(s == end_pos) {

            //TODO: Clean up everything
            return "Hit end of expression looking for closing '`' following 'c' expression code";
        }

        if(*s == '!') {

            expr.typeCode = 'n';
            s++;
    
            if(s == end_pos) {

                //TODO: Clean up everything
                return "No condition following '!' in 'c' template expression conditional";
            }
        }

        for(len = 0; &s[len] != end_pos; len++) if(s[len] == '`') break;

        if(&s[len] == end_pos) {

            //TODO: Clean up everything
            return "Hit end of expression looking for closing '`' following 'c' expression code";
        }

        if((error = String_sliceCString(s, &s[len], &expr.sourcePath)) != 0) {
    
            //TODO: Clean up everything
            return error;
        }

        s = &s[len + 1];

        if(s == end_pos) {

            //TODO: Clean up everything
            return "Hit end of expression looking for template body in 'c' expression code";
        }

        if((error = Template_compile(config, &s, &expr.template)) != 0) {

            //TODO: Clean up everything
            return error;
        }

        if(*s != '`') {

            //TODO: Clean up everything
            return "Expected closing '`' following 'c' template expression body";
        }

        s++;
    }
    
    //Template Format
    //t<source_path>`<template_name>`
    if(expr.typeCode == 't') {
 
        int len = 0;

        for(len = 0; &s[len] != end_pos; len++) if(s[len] == '`') break;

        if(&s[len] == end_pos) {

            //TODO: Clean up everything
            return "Hit end of expression looking for opening '`' following 't' expression code";
        }

        if((error = String_sliceCString(s, &s[len], &expr.sourcePath)) != 0) {
    
            //TODO: Clean up everything
            return error;
        }
   
        s = &s[len + 1]; 

        if(&s[len] == end_pos) {

            //TODO: Clean up everything
            return "Hit end of expression at beginning of template name in 't' expression code";
        }

        for(len = 0; &s[len] != end_pos; len++) if(s[len] == '`') break;

        if(&s[len] == end_pos) {

            //TODO: Clean up everything
            return "Hit end of expression looking for closing '`' following 't' expression code";
        }

        String* template_name;

        if((error = String_sliceCString(s, &s[len], &template_name)) != 0) {
    
            //TODO: Clean up everything
            return error;
        }

        s = &s[len];

        error = Template_getCompiled(config, template_name, &expr.template);

        String_cleanUp(template_name);

        //TODO: Clean up everything
        
        if(error != 0) return error;

        s++;
    }

    //Integer Format
    //i<attribute_path>
    //
    //String format
    //s<attribute_path>
    //    
    //    attribute_path - path to attribute which will be assumed to be an integer and inserted
    if(expr.typeCode == 'i' || expr.typeCode == 's') {

        if((error = String_sliceCString(s, end_pos, &expr.sourcePath)) != 0) {

            //TODO: Clean up everything
        }

        s = end_pos;
    }

    if((*out_expr = (TemplateExpression*)malloc(sizeof(TemplateExpression))) == 0) {

        //TODO: Clean up everything
        return "Failed to allocate memory for a template expression";
    }

    (**out_expr) = expr;
    *sp = s;

    return 0;
}

char* Template_compile(TemplateConfig* config,  char** template_strp, Template** template) {

    char* template_str = *template_strp;
    char* error;

    *template = (Template*)malloc(sizeof(Template));

    if(*template == 0) return "Failed to allocate memory for a template";
    
    VoidList_init(&(*template)->segments);
    VoidList_init(&(*template)->expressions);

    int state = 0;
    char* end_pos = template_str;
    char* start_pos = template_str;
    String* segment;

    //TODO: We need to figure out a good mechanism for escaping special template chars
    for(; *template_str != 0 && state >= 0; template_str++) {

        switch(state) {

            //Looking for the start of an expression node
            case 0:
                if(*template_str == '{') {
                    state = 1;
                } else if(*template_str == '`') {
                    template_str--;
                    state = -1;
                }
                break;

            //Looking for the second brace in an expression node
            case 1:
                if(*template_str == '{') {
                
                    end_pos = template_str + 1;

                    if((error = String_sliceCString(start_pos, end_pos - 2, &segment)) != 0) {
                        
                        //TODO: Clean up everything
                        return error;
                    }

                    VoidList_add(&(*template)->segments, segment);

                    start_pos = end_pos;

                    state = 2;
                } else {

                    state = 0;
                }
                break;

            //Looking for the closing brace in an expression node
            case 2:
                if(*template_str == '}') {
                    state = 3;
                }
                break;

            case 3:
                if(*template_str == '}') {

                    end_pos = template_str + 1;

                    TemplateExpression* expr;

                    if((error = TemplateExpression_tryParse(config, &start_pos, end_pos - 2, &expr)) != 0) {
                        
                        //TODO: Clean up everything
                        return error;
                    }

                    VoidList_add(&(*template)->expressions, expr);
                    start_pos += 2;
                    end_pos = start_pos;
                    template_str = start_pos - 1;
                } 

                state = 0;

                break;
        }
    }   

    if(state == 0 || state == -1)  {

        if((error = String_sliceCString(start_pos, template_str, &segment)) != 0) {
            
            //TODO: Clean up everything
            return error;
        }

        VoidList_add(&(*template)->segments, segment);
    } else {
        
        //TODO: Clean up everything
        return "Template ended in the middle of a template expression";
    }

    *template_strp = template_str;

    return 0;
}

char* Template_getCompiled(TemplateConfig* config, String* template_name, Template** out_template) {

    char* error;
    TemplateInfo* template_info;

    if((error = TemplateConfig_lookUp(config, template_name, &template_info)) != 0) return error;

    char* template_str = template_info->template;

    if(
        (template_info->compiledTemplate == 0) &&
        ((error = Template_compile(
            config,
            &template_str,
            &template_info->compiledTemplate)) != 0) ) return error;
    
    *out_template = template_info->compiledTemplate;

    return 0;
}

char* Template_printInner(Template* template, int depth) {

    char* error;

    for(int i = 0; i < template->segments.count; i++) {

        String* segment = (String*)template->segments.data[i];
    
        print_indent(depth); printf(
            "Segment[%i] '%.*s'\n",
            i, 
            segment->length,
            segment->data
        );

        if(i == template->expressions.count) continue;

        TemplateExpression* expression = (TemplateExpression*)template->expressions.data[i];

        print_indent(depth); printf("Expression[%i]\n", i);
        print_indent(depth); printf("    typeCode: '%c'\n", expression->typeCode);
        print_indent(depth); printf("    sourcePath: '%.*s'\n",
            expression->sourcePath->length, expression->sourcePath->data);
        print_indent(depth); printf("    template: %s\n",
            expression->template == 0 ? "[none]" : "");

        if(expression->template == 0) continue;

        if((error = Template_printInner(expression->template, depth + 2)) != 0) return error;
    }

    return 0;
}

char* Template_print(Template* template) {

    return Template_printInner(template, 0);
}

char* ASTNode_getChildByPath(ASTNode* in_node, String* path, String** rest_str, ASTNode** out_node) {

    char* number_str;
    char* error;
    int i, j, child_idx;

    *out_node = in_node;

    if(path->length == 0) return 0;

    for(i = 0; i < path->length;) {

        if(path->data[i] != 'c') {

            if(path->data[i] == 'a' && rest_str != 0) break;  

            return "Expected node path segment to begin with a 'c'";
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

char* Template_renderCompiledInner(Template* template, ASTNode* node, String** out_str, int child_index); 

char* TemplateExpression_render(
    TemplateExpression* expression, ASTNode* node, String** out_str, int child_index);

char* IntegerTemplateExpression_render(
    TemplateExpression* expression, ASTNode* node, String** out_str, int child_index) {

    char* error;
    void* attribute_ptr;

    if((error = ASTNode_getAttributeByPath(node, expression->sourcePath, &attribute_ptr)) != 0) {

        return error;
    }

    size_t attribute_val = (size_t)attribute_ptr;
    char num_buf[50] = {0};

    sprintf(num_buf, "%li", attribute_val);
    String* num_str = String_new(num_buf);
    String_append(*out_str, num_str);
    String_cleanUp(num_str);

    return 0;
}

char* StringTemplateExpression_render(
    TemplateExpression* expression, ASTNode* node, String** out_str, int child_index) {

    return "String template rendering not implemented";
}

char* ReferenceTemplateExpression_render(
    TemplateExpression* expression, ASTNode* node, String** out_str, int child_index) { 

    char* error;
    ASTNode* source_node;

    if((error = ASTNode_getChildByPath(node, expression->sourcePath, 0, &source_node)) != 0) {

        return error;
    }
   
    if((error = Template_renderCompiledInner(
        expression->template, source_node, out_str, child_index)) != 0) {

        return error;
    }

    return 0;
}

char* ExpansionTemplateExpression_render(
    TemplateExpression* expression, ASTNode* node, String** out_str, int child_index) {

    char* error;
    ASTNode* target_node;

    if((error = ASTNode_getChildByPath(node, expression->sourcePath, 0, &target_node)) != 0) return error;

    if(
        expression->sourcePath->length > 0 &&
        expression->sourcePath->data[expression->sourcePath->length - 1] == 'c'
    ) {

        if((error = Template_renderCompiledInner(expression->template, target_node, out_str, 0)) != 0) {

            return error;
        } else {

            return 0;
        }
    }

    for(int i = 0; i < target_node->childCount; i++) {

         if((error = Template_renderCompiledInner(
            expression->template, target_node->children[i], out_str, i)) != 0) {

            return error;
        }
    }

    return 0;
}

char* ConditionalTemplateExpression_render(
    TemplateExpression* expression, ASTNode* node, String** out_str, int child_index) {

    char* error;
    void* attribute_ptr;

    if(
        expression->sourcePath->length == strlen("first") &&
        strncmp(expression->sourcePath->data, "first", expression->sourcePath->length) == 0
    ) {
        if(
            (expression->typeCode == 'c' && child_index != 0) ||
            (expression->typeCode == 'n' && child_index == 0)
        ) return 0;
    } else {

        if((error = ASTNode_getAttributeByPath(node, expression->sourcePath, &attribute_ptr)) != 0) {

            return error;
        }
     
        size_t attribute_val = (size_t)attribute_ptr;

        if(
            (expression->typeCode == 'c' && attribute_val == 0) ||
            (expression->typeCode == 'n' && attribute_val != 0)
        ) return 0;
    }

    return Template_renderCompiledInner(expression->template, node, out_str, child_index);
}

char* TemplateExpression_render(
    TemplateExpression* expression, ASTNode* node, String** out_str, int child_index) {

    if(expression->typeCode == 'i')
        return IntegerTemplateExpression_render(expression, node, out_str, child_index);

    if(expression->typeCode == 's')
        return StringTemplateExpression_render(expression, node, out_str, child_index);

    if(expression->typeCode == 't')
        return ReferenceTemplateExpression_render(expression, node, out_str, child_index);

    if(expression->typeCode == 'e')
        return ExpansionTemplateExpression_render(expression, node, out_str, child_index);

    if(expression->typeCode == 'c' || expression->typeCode == 'n')
        return ConditionalTemplateExpression_render(expression, node, out_str, child_index);

    return "Encountered an unknown expression type when rendering template";
}

char* Template_renderCompiledInner(Template* template, ASTNode* node, String** out_str, int child_index) {

    char* error;

    for(int i = 0; i < template->segments.count; i++) {

        String* segment = (String*)template->segments.data[i];
    
        String_append(*out_str, segment);

        if(i == template->expressions.count) continue;

        TemplateExpression* expression = (TemplateExpression*)template->expressions.data[i];

        if((error = TemplateExpression_render(
            (TemplateExpression*)template->expressions.data[i],
            node,
            out_str,
            child_index)) != 0) return error;
    }

    return 0;
}

char* Template_renderCompiled(Template* template, ASTNode* node, String** out_str) {

    char* error;
    
    *out_str = String_new(0);

    if(out_str == 0) return "Unable to allocate memory for template output string";

    if((error = Template_renderCompiledInner(template, node, out_str, 0)) != 0) {

        String_cleanUp(*out_str);

        return error;
    }

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

    VoidList lambdas;

    char* error = ASTTree_findAll(node, ASTNode_IsLambda, &lambdas);

    if(error != 0) {

        printf("Error: %s\n", error);

        VoidList_cleanUp(&lambdas);
    }

    for(int i = 0; i < lambdas.count; i++) {

        String* code_str;

        error = ASTNode_renderTemplate(lambdas.data[i], config, &code_str);

        if(error != 0) {

            VoidList_cleanUp(&lambdas);

            return error;
        }
 
        fprintf(out_file, "%.*s", code_str->length, code_str->data);

        String_cleanUp(code_str);
    }

    VoidList_cleanUp(&lambdas);

    return 0;
}

char* Expression_tryParse(FILE* in_file, ASTNode** node);

void ASTModuleNode_print(ASTNode* node, int depth) {

    ASTModuleNode* module_node = (ASTModuleNode*)node;
    
    print_indent(depth); printf("- Module\n");

    for(int i = 0; i < module_node->childCount; i++) {
        
        ASTNode_print(module_node->children[i], depth + 1);
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

    ASTSymbolNode* symbol = (ASTSymbolNode*)node;

    print_indent(depth); printf("- Symbol \n");
    print_indent(depth); printf("  Text: %.*s\n", symbol->text->length, symbol->text->data);
}

void String_cleanUp(String*);

void ASTSymbolNode_cleanUp(ASTNode* node) {

    ASTSymbolNode* symbol_node = (ASTSymbolNode*)node;

    String_cleanUp(symbol_node->text);
}

void ASTOperatorNode_print(ASTNode* node, int depth) {

    ASTOperatorNode* operator_node = (ASTOperatorNode*)node;

    print_indent(depth); printf("- Operator\n");
    print_indent(depth); printf("  Operation: %s\n", OperatorString[operator_node->operatorType]);
    print_indent(depth); printf("  LeftExpr:\n");
    ASTNode_print(operator_node->ON_LEFT_EXPR, depth + 1);
    print_indent(depth); printf("  RightExpr:\n");
    ASTNode_print(operator_node->ON_RIGHT_EXPR, depth + 1);
}

void ASTOperatorNode_cleanUp(ASTNode* node) { }

void ASTLambdaNode_print(ASTNode* node, int depth) {

    ASTLambdaNode* lambda_node = (ASTLambdaNode*)node;

    print_indent(depth); printf("- Lambda\n");
    print_indent(depth); printf("  Parameters:\n");

    ASTNode_print(lambda_node->LN_PARAMS, depth + 1);

    print_indent(depth); printf("  Body:\n");

    ASTNode_print(lambda_node->LN_EXPR, depth + 1);
}

void ASTLambdaNode_cleanUp(ASTNode* node) { }

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

    error = ASTNode_create(node, Symbol, 0, 0, sizeof(ASTSymbolNode));

    if(*node == 0) {

        fsetpos(in_file, &original_position);

        String_cleanUp(symbol_text);

        return "Couldn't allocate memory for ast symbol";
    }

    ((ASTSymbolNode*)*node)->text = symbol_text;

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

char* Declaration_tryParse(FILE* in_file, ASTNode** node); 
char* Expression_tryParse(FILE* in_file, ASTNode** node);

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

char* Expression_tryParse(FILE* in_file, ASTNode** node) {

    //TEMP
    //printf("Trying to parse an expression\n");

    char* error;
    
    if(Lambda_tryParse(in_file, node) == 0) return 0;
    if((error = Operator_tryParse(in_file, node)) == 0) return 0;

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

    //TODO: Actually parse command line args as described
    FILE* out_file = fopen(argv[2], "w");

    error_message = ASTNode_writeOut(out_file, &CTemplateConfig, module_ast);

    if(error_message != 0)
        printf("Writing out failed: %s\n", error_message);

    fclose(out_file);

    ASTNode_cleanUp(module_ast);

    return 0;
}
