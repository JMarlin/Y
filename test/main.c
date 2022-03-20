#include "test.y.c"

int main(int argc, char* argv) {

    _y_init();

    printf("1 add 2 = %i\n", add(1, 2));

    return 0;
}
