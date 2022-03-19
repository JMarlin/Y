#ifndef PARSE_H
#define PARSE_H

char* Declaration_tryParse(FILE* in_file, ASTNode** node); 
char* Expression_tryParse(FILE* in_file, ASTNode** node);

#endif //PARSE_H
