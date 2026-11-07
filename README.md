# SSL Interpreter
Simple-Scripting-Language made to be intuitive to use. At first it was made as an AST interpreter, which is the easiest to implement but also one of the slowest and most fragile ones. It's easy to crash it with recursive functions. Later on I tried to change it to a bytecode VM interpreter, which is a better choice and that's how Python and Lua work. But that failed and I've long since abandoned this project for other languages, which were way more successful. On November 2026 I reverted to the older, working commit.

This is a half-finished project. Feel free to still play around with it but note that it might contain bugs and inconsistencies. Here's a little file of some of the features present in SSL:
```
fn factorial(n)
   if n <= 1 return 1 end
   return n * factorial(n - 1)
end

x :: 20 // constant
y := 5 // variable
println(x, y)

println(factorial(y))

match y
case 5 println('it is 5')
case 1 println('it is 1')
else println('it is something else...')
end

println('array: ')
for v, i in [5, 4, 3, 2, 1, 0] reversed
   printfln('array[{}] = {}', i, v)
end

fun :: lambda(n)
   return n * n
end

println('Lambda:', fun(5), fun(16))
```

## Building
This project depends on C++20 and fmt library, which is fetched automatically by CMake. That is the preferred way to build this project.
```bash
cmake -B build
cmake --build build
```
The executable will be located in `build/ssl`.

## Usage
The interpreter has no options and you can only pass code or an input file.
```bash
./build/ssl 'println("Hello, World!")'
./build/ssl 'file.ssl'
```

## Documentation

## Credits
This project was made by Daniel Vishnevsky and is licensed under MIT License.
