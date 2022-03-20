#ifndef CTEMPLATE_H
#define CTEMPLATE_H

TemplateConfig CTemplateConfig = {
    {
    "module", //Module
    "", //Declaration
    "", //Parameter
    "", //ParameterList
    "", //Operator
    "lambda_declaration", //Lambda
    "" //Symbol
    },
    4,
    {
        {
            "module",
            "{{er`{{c`pred`{{t`lambda_declaration`}}`}}`}}\n"
            "{{tc`global_declarations`}}\n"
            "void _y_init() { } \n", 0, 
            ASTNode_IsLambda
        }, 
        {
            "global_declarations",
            "{{ec`{{c`pred`//TODO: declarations\n`}}`}}", 0,
            ASTNode_IsDeclaration
        },
        {
            "lambda_declaration",
            "typedef int (*Lambda{{ia0}}Type)({{ec0`{{c`!first`, `}}int`}});\n"
            "int Lambda{{ia0}}({{ec0`{{c`!first`, `}}int {{sc0a0}}`}}) { return {{tc1`expression`}}; }\n", 0, 0
        },
        {
            "expression",
            "10", 0, 0
        }
    }
};

#endif //CTEMPLATE_H
