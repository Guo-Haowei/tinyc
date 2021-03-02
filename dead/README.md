# toy_c

toy_c is a toy c compiler. (WIP)

## Prerequistes

Platform: windows

`nasm`: install [nasm](https://www.nasm.us/) to compile `.asm` code to `.obj` file.

`gcc`: install [mingw](http://www.mingw.org/) to compile `.obj` to executable.

## Task

- [ ] Generate IR for register allocation and other optimizations
- [ ] Use linear allocator for simplicity
- [ ] Refactor preprocess steps.

### Compiler

- [ ] operators
  - [x] arith `+`, `-`, `*`, `/`, `%`
  - [x] relational `==`, `!=`, `>=`, `<=`, `>`, `<`
  - [x] logical `!`, `&&`, `||`
  - [x] increment `++`, `--`
  - [x] assignment `=`
  - [ ] assignment `+=`, `-=`, `*=`, `/=`, `%=`, `>>=`, `<<=`
  - [ ] bitwise `&`, `|`, `~`, `>>`, `<<`
- [ ] statements
  - [x] branch
    - [x] `if`, `else`
    - [x] ternary `?:`
  - [ ] loop
    - [x] `while`
    - [x] `for`
    - [ ] `do while`
  - [ ] jump
    - [x] `continue`
    - [x] `break`
    - [ ] `switch`
    - [ ] `case`
- [ ] data types
  - [ ] type validation
  - [ ] `sizeof`
  - [x] `int`
  - [x] pointer `*`
  <!-- - [ ] `unsigned` -->
  - [ ] `char`
  - [ ] `string`
  - [ ] `enum`
  - [ ] `struct`
- [x] variable
- [x] scope
- [x] function
- [ ] address
  - [x] reference `&`
  - [x] dereference `*`
  - [x] subscript `[]`
- [ ] `struct`
  - [ ] `.`
  - [ ] `->`
- [ ] va_args
