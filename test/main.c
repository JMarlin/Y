#include <stdio.h>
#include "test.y.c"

int main(int argc, char* argv) {

    printf("1 add 2 = %i\n", addNumbers(1, 2));

    return 0;
}
