#ifndef TEMPLATE_H
#define TEMPLATE_H

struct Template_s;
struct TemplateInfo_s;
struct TemplatePredicate_s;
struct TemplateConfig_s;
struct TemplateExpression_s;
struct RecursiveRenderArgs_s;

#include "ast.h"
#include "voidlist.h"
#include "string.h"

typedef struct Template_s {
    VoidList segments;
    VoidList expressions;
    struct TemplateInfo_s* info;
} Template;

typedef struct TemplateInfo_s {
    char* templateName;
    char* template;
    Template* compiledTemplate;
    ASTNodePredicate predicate;
} TemplateInfo;

typedef struct TemplatePredicate_s {
    char* predicateName;
} TemplatePredicate;

typedef struct TemplateConfig_s {
    char* baseTemplateName[ASTNodeTypeCount];
    int templateCount;
   TemplateInfo templateList[];
} TemplateConfig;

typedef struct TemplateExpression_s {
    char typeCode;
    String* sourcePath;
    Template* template;
} TemplateExpression;

typedef struct RecursiveRenderArgs_s {
    Template* template;
    String** outStr;
    int index;
} RecursiveRenderArgs;

char* Template_compile(TemplateConfig* config, TemplateInfo* info, char** template_strp,
    Template** template);

char* Template_getCompiled(TemplateConfig* config, String* template_name, Template** out_template);

char* Template_renderCompiledInner(Template* template, struct ASTNode_s* node, String** out_str,
    int child_index); 

char* Template_renderCompiled(Template* template, struct ASTNode_s* node, String** out_str);

char* TemplateExpression_render(Template* template, TemplateExpression* expression,
    struct ASTNode_s* node, String** out_str, int child_index);

#endif //TEMPLATE_H
