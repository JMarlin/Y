typedef int (*Lambda0Type)(int, int);
int Lambda0(int a, int b) { return a + b; }
typedef int (*Lambda1Type)(int);
int Lambda1(int i) { return i + 1; }

#include <stdio.h>
int main(int argc, char* argv[]) {
Lambda0Type add = Lambda0;
Lambda1Type inc = Lambda1;



;;printf("add(1, 2) = %dn", add(1, 2));printf("inc(2) = %dn", inc(2));
}
