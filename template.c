#include "template.h"
#include "helpers.h"
#include <string.h>
#include <stdlib.h>

char* TemplateConfig_lookUp(TemplateConfig* config, String* template_name,
    TemplateInfo** template_info) {

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

char* TemplateExpression_tryParse(TemplateConfig* config, TemplateInfo* info, char** sp,
    char* end_pos, TemplateExpression** out_expr) {

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
    //                if the path ends in an 's', the inner template is expanded only for the one child
    //                if the path ends in an 'r', the inner template is expanded recursively for every child in the tree
    //                if the path does not end in an 's' or an 'r', the inner template is expanded for each
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

        if((error = Template_compile(config, info, &s, &expr.template)) != 0) {

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
    //c`first | !first | pred | !pred | <attribute_path> | !<attribute_path>`text_expr`
    //Conditions:
    //    first - true when this is either a standalone node or the first in an iteration
    //    !first - true when this is not the first node in an iteration
    //    pred/!pred - true/false when the attached predicate function evaluates true
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

        if((error = Template_compile(config, info, &s, &expr.template)) != 0) {

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

char* Template_compile(TemplateConfig* config, TemplateInfo* info, char** template_strp, Template** template) {

    char* template_str = *template_strp;
    char* error;

    *template = (Template*)malloc(sizeof(Template));

    if(*template == 0) return "Failed to allocate memory for a template";

    (*template)->info = info;
    
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

                    if((error = TemplateExpression_tryParse(config, info, &start_pos, end_pos - 2, &expr)) != 0) {
                        
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
            template_info,
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

char* IntegerTemplateExpression_render(Template* template, TemplateExpression* expression,
    ASTNode* node, String** out_str, int child_index) {

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

char* StringTemplateExpression_render(Template* template, TemplateExpression* expression,
    ASTNode* node, String** out_str, int child_index) {

    char* error;
    void* attribute_ptr;

    if((error = ASTNode_getAttributeByPath(node, expression->sourcePath, &attribute_ptr)) != 0) {

        return error;
    }

    String* attribute_str = (String*)attribute_ptr;

    String_append(*out_str, attribute_str);

    return 0;
}

char* ReferenceTemplateExpression_render(Template* template, TemplateExpression* expression,
    ASTNode* node, String** out_str, int child_index) { 

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

char* Template_renderDescender(ASTNode* node, void* void_args) {

    RecursiveRenderArgs* args = (RecursiveRenderArgs*)void_args;

    return Template_renderCompiledInner(args->template, node, args->outStr, args->index);
}

char* ExpansionTemplateExpression_render(Template* template, TemplateExpression* expression,
    ASTNode* node, String** out_str, int child_index) {

    char* error;
    ASTNode* target_node;

    if((error = ASTNode_getChildByPath(node, expression->sourcePath, 0, &target_node)) != 0) return error;

    if(
        expression->sourcePath->length > 0 &&
        expression->sourcePath->data[expression->sourcePath->length - 1] == 'c'
    ) {

        if((error = Template_renderCompiledInner(expression->template, target_node, out_str, 0)) != 0) {

            return error;
        } 

        return 0;
    }

    if(
        expression->sourcePath->length > 0 &&
        expression->sourcePath->data[expression->sourcePath->length - 1] == 'r'
    ) {

        RecursiveRenderArgs args = { expression->template, out_str, 0 };

        if((error = ASTNode_forAll(target_node, Template_renderDescender, &args)) != 0) {

            return error;
        }

        return 0;
    }

    for(int i = 0; i < target_node->childCount; i++) {

         if((error = Template_renderCompiledInner(
            expression->template, target_node->children[i], out_str, i)) != 0) {

            return error;
        }
    }

    return 0;
}

char* ConditionalTemplateExpression_render(Template* template, TemplateExpression* expression,
    ASTNode* node, String** out_str, int child_index) {

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
    } else if( 
        expression->sourcePath->length == strlen("pred") &&
        strncmp(expression->sourcePath->data, "pred", expression->sourcePath->length) == 0
    ) {

        if(template->info->predicate == 0) return "Predicate specified in template, but predicate pointer is null";

        int result = template->info->predicate(node);

        if(
            (expression->typeCode == 'c' && !result) ||
            (expression->typeCode == 'n' && result)
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

char* TemplateExpression_render(Template* template, TemplateExpression* expression,
    ASTNode* node, String** out_str, int child_index) {

    if(expression->typeCode == 'i')
        return IntegerTemplateExpression_render(template, expression, node, out_str, child_index);

    if(expression->typeCode == 's')
        return StringTemplateExpression_render(template, expression, node, out_str, child_index);

    if(expression->typeCode == 't')
        return ReferenceTemplateExpression_render(template, expression, node, out_str, child_index);

    if(expression->typeCode == 'e')
        return ExpansionTemplateExpression_render(template, expression, node, out_str, child_index);

    if(expression->typeCode == 'c' || expression->typeCode == 'n')
        return ConditionalTemplateExpression_render(template, expression, node, out_str, child_index);

    return "Encountered an unknown expression type when rendering template";
}

char* Template_renderCompiledInner(Template* template, ASTNode* node, String** out_str,
    int child_index) {

    char* error;

    for(int i = 0; i < template->segments.count; i++) {

        String* segment = (String*)template->segments.data[i];
    
        String_append(*out_str, segment);

        if(i == template->expressions.count) continue;

        TemplateExpression* expression = (TemplateExpression*)template->expressions.data[i];

        if((error = TemplateExpression_render(
            template,
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


