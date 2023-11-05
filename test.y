//TODO: Implement expression grouping
//var addThenMult = (var a, var b, var c) => (a + b) * c;

//TODO: Implement comparison operators
//var lessThan = (var a, var b) => a < b;

//TODO: Implement ternary
//var fib = (var i) => i < 3 ? 1 : (fib(i - 2) + fib(i - 1));

//printf("fib(1) = %d\n", fib(1));
//printf("fib(2) = %d\n", fib(2));
//printf("fib(3) = %d\n", fib(3));
//printf("fib(4) = %d\n", fib(4));

var inc = (var i) => add(i, 1);
var add = (var a, var b) => a + b;

printf("add(inc(1), 2) = %d\n", add(inc(1), 2));

//TODO: segregate assignment from declaration
//add = (var a, var b) => 0;
//printf("add(inc(1), 2) = %d\n", add(inc(1), 2));
