# Lua Compiler in C

This project is a tiny Lua compiler implemented in C. It compiles Lua source code into a custom bytecode format and executes it on a virtual machine.

This repository was created with the help of the [Gemini CLI](https://github.com/google/gemini-cli).

## Usage

To compile and run a Lua file, use the following command:

```bash
./luac <source_file>
```

## Building

To build the compiler, you can use the provided Makefile.

### Prerequisites

- GCC compiler
- Make

### Compilation

To build the compiler, run the `make` command:

```bash
make
```

This will create the `luac` executable in the root directory.

To build the compiler with debug tracing enabled, run:

```bash
make debug
```

### Cleaning

To clean up the build artifacts, run:
```bash
make clean
```

## Testing

The project includes a test suite. To run the tests, execute the following command:

```bash
make test
```

This will run the `run_tests.sh` script, which compares the output of the compiler with the expected output for a set of test cases.
