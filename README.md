# The GEN Programming Language Interpreter
Interpreter for the GEN programming language written in C.

## About
Below is an example code of the GEN programming language representing a recursive fibonacci implementation without caching.
```
func fibonacci_recursive(var number) {
    if (number <= 1) {
        return number;
    }

    return fibonacci_recursive(number - 1) + fibonacci_recursive(number - 2);
}

func fibonacci(var n) {
    var i = 0;

    while (i < n) {
        print fibonacci_recursive(i);
        var i = i + 1;
    }
}

func main() {
    var n = 30;
    fibonacci(n);
}
```

by [Tomas Boda](https://github.com/TomasBoda)
