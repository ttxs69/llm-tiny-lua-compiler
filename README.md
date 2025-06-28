# Lua Compiler in C

[![C CI](https://github.com/ttxs69/llm-tiny-lua-compiler/actions/workflows/ci.yml/badge.svg)](https://github.com/ttxs69/llm-tiny-lua-compiler/actions/workflows/ci.yml)

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

To run the tests with debug tracing enabled, pass the `ARGS` variable to the `make` command with the desired flags.

- `-p`: Enable parser tracing.
- `-c`: Enable codegen tracing.
- `-e`: Enable execution tracing.

For example, to run the tests with all tracing enabled, use:

```bash
make test ARGS="-pce"
```
