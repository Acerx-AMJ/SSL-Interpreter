# SSL Interpreter
Simple-Scripting-Language made to be intuitive to use. At first it was made as an AST interpreter, which is the easiest to implement but also one of the slowest and most fragile ones. It's easy to crash it with recursive functions. Later on I tried to change it to a bytecode VM interpreter, which is a better choice and that's how Python and Lua work. But that failed and I've long since abandoned this project for other languages, which were way more successful.

If you'd like to check out the AST interpreter, see [this older commit](https://github.com/Acerx-AMJ/SSL-Interpreter/tree/8b037d299d9d7f8d3fd050767b84e580cec1d2e6). If you want to play around with this half-finished interpreter instead and see how the base works, feel free to do so.

## Building
This project depends on C++20 and fmt library, which is fetched automatically by CMake. That is the preferred way to build this project.
```bash
cmake -B build
cmake --build build
```
The executable will be located in `build/ssl`.

## Usage
The interpreter has no options and you can only pass code or an input file. Due to the state of the project, it only works as a calculator. And to print the value you have to use `return`. Weird, I know.
```bash
./build/ssl 'return 20 + 3'
./build/ssl 'return (2 + 3) * 4'
```

## Credits
This project was made by Daniel Vishnevsky and is licensed under MIT License.
