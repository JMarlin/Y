#ifndef TEMPLATE_H
#define TEMPLATE_H

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

char* Template_renderCompiledInner(Template* template, ASTNode* node, String** out_str,
    int child_index); 

char* TemplateExpression_render(Template* template, TemplateExpression* expression,
    ASTNode* node, String** out_str, int child_index);


#endif //TEMPLATE_H
