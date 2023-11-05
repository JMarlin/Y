var inc = (var i) => add(i, 1);
var add = (var a, var b) => a + b;

printf("add(inc(1), 2) = %d\n", add(inc(1), 2));
