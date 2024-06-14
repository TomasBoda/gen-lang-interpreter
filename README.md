# The GEN Programming Language
GEN is a general-purpose, procedural, dynamically typed programming language based on a bytecode virtual machine interpreter written in C.

## About
Below is an example code of the GEN programming language representing a recursive fibonacci implementation without caching.
```
func fibonacci(var n) {
    var i = 0;

    while (i < n) {
        var index = i + 1;
        var value = fibonacci_recursive(i);

        fibonacci_print(index, value);
        i = i + 1;
    }
}

func fibonacci_recursive(var number) {
    if (number <= 1) {
        return number;
    }

    return fibonacci_recursive(number - 1) + fibonacci_recursive(number - 2);
}

func fibonacci_print(var index, var value) {
    print index;
    print ": ";
    print value endl;
}

func main() {
    var n = 30;
    fibonacci(n);
}
```

## Building & Running
To build and run the interpreter with the example program in the `program.gen` file, run the prepared bash script.
```bash
./interpret.sh
```

by [Tomas Boda](https://github.com/TomasBoda)
