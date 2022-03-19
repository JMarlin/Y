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
    2,
    {
        {
            "module",
            "{{er`{{c`pred`{{t`lambda_declaration`}}`}}`}}", 0, 
            ASTNode_IsLambda
        }, 
        {
            "lambda_declaration",
            "typedef int (*Lambda{{ia0}}Type)({{ec0`{{c`!first`, `}}int`}});\n"
            "int Lambda{{ia0}}({{ec0`{{c`!first`, `}}int {{sc0a0}}`}}) { }\n", 0, 0
        }
    }
};

#endif //CTEMPLATE_H
