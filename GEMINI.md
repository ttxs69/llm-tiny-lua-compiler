# Project Goal

My goal is to implement a tiny Lua compiler in C with the help of an LLM.

## Acceptance Criteria

*   The compiler takes a Lua file as input.
*   It uses its own bytecode format and a virtual machine for execution.
*   It is not a transpiler (does not generate C code).
*   The LLM should help with the implementation and testing of the compiler.

## Instructions for the LLM

*   When I ask you to add a new feature, you should:
    1.  Add a new test case for the feature in the `test` directory.
    2.  Run the tests to see the new test fail.
    3.  Implement the feature in the compiler.
    4.  Run the tests again to see the new test pass.
*   When I ask you to refactor the code, you should:
    1.  Identify areas for improvement.
    2.  Refactor the code to improve readability, maintainability, and error handling.
    3.  Run the tests to ensure that the refactoring did not break anything.
*   All shell commands executed by the agent must use a timeout to prevent hangs. For example: `timeout 30s make test`.
