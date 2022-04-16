typedef int (*Lambda0Type)(int, int);
int Lambda0(int a, int b) { return a + b; }

int main(int argc, char* argv[]) {
Lambda0Type add = Lambda0;

}
