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
    "", //Symbol
    "", //Invocation
    "", //ArgumentList
    "", //StringLiteral
    "", //NumberLiteral
    },
    15,
    {
        {
            "module",
            "{{er`{{c`pred`{{t`lambda_declaration`}}`}}`}}\n"
            "#include <stdio.h>\n"
            "int main(int argc, char* argv[]) {\n"
            "{{tc`global_declarations`}}\n"
            "{{tc`global_expressions`}}\n"
            "}\n", 0, 
            ASTNode_IsLambda
        }, 
        {
            "global_declarations",
            "{{e`{{c`pred`Lambda{{ic1a0}}Type {{sc0a0}} = Lambda{{ic1a0}};`}}\n`}}", 0,
            ASTNode_IsDeclaration
        }, 
        {
            "global_expressions",
            "{{e`{{t`expression`}};`}}", 0, 0
        },
        {
            "lambda_declaration",
            "typedef int (*Lambda{{ia0}}Type)({{ec0`{{c`!first`, `}}int`}});\n"
            "int Lambda{{ia0}}({{ec0`{{c`!first`, `}}int {{sc0a0}}`}}) { return {{tc1`expression`}}; }\n", 0, 0
        },
        {
            "expression",
            "{{tc`operator_expression`}}{{tc`symbol_expression`}}{{tc`invocation_expression`}}{{tc`string_expression`}}{{tc`number_expression`}}", 0, 0
        },
        {
            "string_expression",
            "{{c`pred`\"{{sa0}}\"`}}", 0,
            ASTNode_IsStringLiteral
        },
        {
            "number_expression",
            "{{c`pred`{{ia0}}`}}", 0,
            ASTNode_IsNumberLiteral
        },
        {
            "symbol_expression",
            "{{c`pred`{{sa0}}`}}", 0,
            ASTNode_IsSymbol
        },
        {
            "operator_expression",
            "{{c`pred`{{tc0`expression`}} {{tc`operator`}} {{tc1`expression`}}`}}", 0, 
            ASTNode_IsOperator
        },
        {
            "operator",
            "{{tc`add_operator`}}{{tc`sub_operator`}}{{tc`mul_operator`}}{{tc`div_operator`}}", 0, 0
        },
        {
            "add_operator",
            "{{c`pred`+`}}", 0,
            ASTOperatorNode_OperatorIsAdd
        },
        {
            "sub_operator",
            "{{c`pred`-`}}", 0,
            ASTOperatorNode_OperatorIsSub
        },
        {
            "mul_operator",
            "{{c`pred`*`}}", 0,
            ASTOperatorNode_OperatorIsMul
        },
        {
            "div_operator",
            "{{c`pred`/`}}", 0,
            ASTOperatorNode_OperatorIsDiv
        },
        {
            "invocation_expression",
            "{{c`pred`{{tc0`symbol_expression`}}({{ec1`{{c`!first`, `}}{{t`expression`}}`}})`}}", 0,
            ASTNode_IsInvocation
        }
    }
};

#endif //CTEMPLATE_H
