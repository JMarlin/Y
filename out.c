typedef int (*Lambda0Type)(int);
typedef int (*Lambda1Type)(int, int);

Lambda0Type inc;
Lambda1Type add;


int Lambda0(int i) { return add(i, 1); }
int Lambda1(int a, int b) { return a + b; }

#include <stdio.h>
int main(int argc, char* argv[]) {
inc = Lambda0;
add = Lambda1;


;;printf("add(inc(1), 2) = %d\n", add(inc(1), 2));
}
