all: yc

yc: main.o helpers.o ast.o parse.o template.o string.o voidlist.o
	gcc -o yc main.o helpers.o ast.o parse.o template.o string.o voidlist.o -g

main.o: main.c ast.h parse.h ctemplate.h
	gcc -c -o main.o main.c -g

helpers.o: helpers.c helpers.h
	gcc -c -o helpers.o helpers.c -g

ast.o: ast.c ast.h helpers.h template.h string.h voidlist.h
	gcc -c -o ast.o ast.c -g

parse.o: parse.c parse.h helpers.h ast.h string.h voidlist.h
	gcc -c -o parse.o parse.c -g

template.o: template.c template.h ast.h string.h voidlist.h
	gcc -c -o template.o template.c -g

string.o: string.c string.h
	gcc -c -o string.o string.c -g

voidlist.o: voidlist.c voidlist.h
	gcc -c -o voidlist.o voidlist.c -g

