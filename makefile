all: yc

yc: main.o helpers.o ast.o parse.o template.o string.o voidlist.o
	gcc -o yc main.o helpers.o ast.o parse.o template.o string.o voidlist.o

main.o: main.c ast.h parse.h ctemplate.h
	gcc -c -o main.o main.c

helpers.o: helpers.c helpers.h
	gcc -c -o helpers.o helpers.c

ast.o: ast.c ast.h helpers.h template.h string.h voidlist.h
	gcc -c -o ast.o ast.c

parse.o: parse.c parse.h helpers.h ast.h string.h voidlist.h
	gcc -c -o parse.o parse.c

template.o: template.c template.h ast.h string.h voidlist.h
	gcc -c -o template.o template.c

string.o: string.c string.h
	gcc -c -o string.o string.c

voidlist.o: voidlist.c voidlist.h
	gcc -c -o voidlist.o voidlist.c

