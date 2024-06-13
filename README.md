# The GEN Programming Language Interpreter
Interpreter for the GEN programming language written in C.

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
    print index + 1;
    print ": ";
    print value endl;
}

func main() {
    var n = 30;
    fibonacci(n);
}
```

## Building & Running
To build and run the interpreter with the example source code in the `program.gen` file, run the prepared bash script:
```bash
./interpret.sh
```
The script will firstly build the sources and then run the `program.gen` file.

by [Tomas Boda](https://github.com/TomasBoda)
