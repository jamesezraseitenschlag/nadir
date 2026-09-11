# Nadir - Native Salesforce Apex Runtime Engine

Nadir is a lightweight, high-performance, cross-platform interpreter and runtime engine for the Salesforce Apex programming language, implemented in ISO C11 with zero external runtime dependencies.

---

## Dependencies & Prerequisites

Nadir is deliberately designed without third-party runtime library requirements. All required development and testing tools are linked below:

### Core Runtime & Build Dependencies
- **C Standard Library**: Standard C11 runtime ([ISO/IEC 9899:2011](https://www.iso.org/standard/57853.html)) with POSIX math library ([`libm`](https://man7.org/linux/man-pages/man3/math.h.0p.html)).
- **Build System**: [CMake](https://cmake.org/) (version 3.15 or newer).
- **C Compiler**: Any standard-compliant C11 compiler:
  - [GCC](https://gcc.gnu.org/) (GNU Compiler Collection, version 7.0+)
  - [Clang / LLVM](https://clang.llvm.org/) (version 6.0+)
  - [Microslop Visual C++ (MSVC)](https://visualstudio.microsoft.com/) (Visual Studio 2019 or newer)
  - [Zig](https://ziglang.org/) (`zig cc` toolchain for instant cross-compilation)
  - [Apple Clang](https://developer.apple.com/xcode/) (Xcode 11 or newer)

### Test Harness & Validation Dataset
- **Test Automation**: [Python 3](https://www.python.org/) (version 3.7 or newer) using only the Python standard library (`subprocess`, `glob`, `os`, `sys`).
- **Benchmark Suite**: [Salesforce Trailhead Apps Apex Recipes](https://github.com/trailheadapps/apex-recipes) for language conformance testing.
- **Language Reference**: [Salesforce Apex Developer Guide](https://developer.salesforce.com/docs/atlas.en-us.apexcode.meta/apexcode/).

### Grammar Specifications & Origins
Credit where credit is due — the reference ANTLR4 grammars in `grammars/` were sourced and adapted from:
- [ANTLR grammars-v4 Community Repository](https://github.com/antlr/grammars-v4) for foundational grammar patterns.
- [Salesforce apex-jorje & PMD Apex Parser](https://github.com/pmd/pmd/tree/master/pmd-apex) for Apex-specific AST structure and keyword mappings.
- Turns out, not having to manually invent language grammar rules from scratch is a surprisingly good strategy.

---

## Architecture Overview

The runtime is structured into modular, decoupled subsystems:

- **Lexer & Tokenizer** ([`src/lexer.c`](file:///C:/Users/James/ARFS/src/lexer.c)): Case-insensitive UTF-8 stream scanner supporting Apex keywords, SOQL operators, annotations, and literals.
- **Recursive-Descent Parser** ([`src/parser.c`](file:///C:/Users/James/ARFS/src/parser.c)): Modular grammar parser with dedicated modules for declarations ([`src/parser_decl.c`](file:///C:/Users/James/ARFS/src/parser_decl.c)), statements ([`src/parser_stmt.c`](file:///C:/Users/James/ARFS/src/parser_stmt.c)), expressions ([`src/parser_expr.c`](file:///C:/Users/James/ARFS/src/parser_expr.c)), and inline SOQL ([`src/parser_soql.c`](file:///C:/Users/James/ARFS/src/parser_soql.c)).
- **Abstract Syntax Tree** ([`src/ast.c`](file:///C:/Users/James/ARFS/src/ast.c)): Arena-managed syntax tree nodes representing OOP constructs, control flow, collections, and database operations.
- **Interpreter & Evaluator** ([`src/eval.c`](file:///C:/Users/James/ARFS/src/eval.c)): Modular tree-walking execution engine with scoped environments, call stack unwinding, and runtime error reporting.
- **Operator Engine** ([`src/eval_op.c`](file:///C:/Users/James/ARFS/src/eval_op.c)): Arithmetic, relational, logical, and assignment operations with type coercion.
- **Standard Library Subsystem** ([`src/eval_sys.c`](file:///C:/Users/James/ARFS/src/eval_sys.c)): Built-in mock implementations for `System`, `Math`, `String`, `JSON`, `Database`, `Test`, `Limits`, `UserInfo`, and `Schema` namespaces.
- **Method & Dispatch Engine** ([`src/eval_call.c`](file:///C:/Users/James/ARFS/src/eval_call.c)): Dynamic dispatch for static class methods, instance methods, and collection manipulation.
- **SObject & Mock Database Engine** ([`src/sobject.c`](file:///C:/Users/James/ARFS/src/sobject.c)): In-memory relational storage providing SObject records, 15/18-character Salesforce ID generation, SOQL querying, savepoints, rollback, and DML transaction tracking.

---

## Project Structure

```text
nadir/
├── CMakeLists.txt             # Cross-platform CMake build configuration
├── LICENSE                    # MIT License
├── README.md                  # Project documentation
├── test_runner.py             # Automated test suite runner
│
├── .github/
│   └── workflows/
│       └── build.yml          # Multi-platform CI/CD and packaging workflow
│
├── include/                   # Header declarations
│   ├── ast.h                  # AST node types and constructors
│   ├── common.h               # Portable platform definitions and string utilities
│   ├── env.h                  # Lexical environment and symbol table
│   ├── eval.h                 # Public interpreter API
│   ├── eval_internal.h        # Internal evaluation and dispatch prototypes
│   ├── lexer.h                # Lexer interface and state
│   ├── parser.h               # Public parser interface
│   ├── parser_internal.h      # Parser subsystem prototypes
│   ├── sobject.h              # SObject schema and in-memory mock database
│   ├── token.h                # Token taxonomy and keyword enumeration
│   └── value.h                # Tagged-union dynamic value model
│
├── src/                       # Source implementation files (C11)
│   ├── ast.c                  # AST allocation and cleanup
│   ├── env.c                  # Environment binding and resolution
│   ├── eval.c                 # Interpreter state and core evaluation dispatch
│   ├── eval_call.c            # Method invocation and static/instance dispatch
│   ├── eval_op.c              # Arithmetic, relational, logical, and assignment ops
│   ├── eval_sys.c             # Standard library namespace implementations
│   ├── lexer.c                # Token scanning and identifier lookup
│   ├── main.c                 # CLI executable entry point and REPL shell
│   ├── parser.c               # Parser initialization and token navigation
│   ├── parser_decl.c          # Classes, interfaces, triggers, and declarations
│   ├── parser_expr.c          # Expression precedence climbing and literals
│   ├── parser_soql.c          # Inline SOQL query parsing
│   ├── parser_stmt.c          # Control flow, loops, and statement parsing
│   ├── sobject.c              # SObject records, DML, and mock SOQL engine
│   ├── token.c                # Token diagnostics and string conversion
│   └── value.c                # Value constructors, collections, and comparisons
│
├── examples/                  # Apex code examples
│   ├── 01_hello_world.apex
│   ├── 02_control_flow.apex
│   ├── 03_collections.apex
│   ├── 04_classes_oop.apex
│   ├── 05_soql_and_dml.apex
│   ├── all_features_test.apex
│   ├── enterprise_order_management.apex
│   └── ultimate_apex_test.apex
│
└── specs/                     # Language specifications and schema definitions
    ├── apex_keywords.json
    ├── apex_language_reference.md
    ├── sobject_schema.json
    └── standard_types.json
```

---

## Building the Project

### Native Build (Linux, macOS, Windows)

```bash
cmake -B build
cmake --build build --config Release
```

The resulting binary will be located at:
- **Linux / macOS**: `build/nadir`
- **Windows**: `build/Release/nadir.exe` (or `build/nadir.exe`)

---

## Generating Distributable Packages

Nadir is configured with [CPack](https://cmake.org/cmake/help/latest/module/CPack.html) to produce self-contained distributable archives (`.zip`, `.tar.gz`, `.7z`):

```bash
# 1. Build the project in Release mode
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 2. Package the release distribution
cpack --config build/CPackConfig.cmake -C Release -B dist
```

Generated distributables in `dist/` include the runtime binary, documentation, license, and standard library examples ready for redistribution.

---

## Cross-Compilation

Nadir relies strictly on standard C11 and portable POSIX/C runtime interfaces, enabling cross-compilation across architectures:

### 1. Cross-compiling with GCC / MinGW (Linux to Windows)
```bash
cmake -B build-win \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc
cmake --build build-win
```

### 2. Cross-compiling with Clang / Zig CC (Multi-target)
```bash
# Target Linux x86_64
zig cc -Iinclude src/*.c -target x86_64-linux-gnu -o nadir_linux -O2 -Wall -lm

# Target Windows x86_64
zig cc -Iinclude src/*.c -target x86_64-windows-gnu -o nadir.exe -O2 -Wall

# Target macOS Apple Silicon (aarch64)
zig cc -Iinclude src/*.c -target aarch64-macos-none -o nadir_macos -O2 -Wall
```

### 3. Cross-compiling to WebAssembly (Emscripten)
```bash
emcmake cmake -B build-wasm
cmake --build build-wasm
```

---

## Usage & Execution Guide

Nadir provides multiple execution modes for running Apex code, from interactive shells to multi-file batch execution and native C API embedding:

### 1. Interactive REPL Mode
Running the binary without arguments opens the interactive shell:

```bash
./build/nadir
```

```apex
Nadir v1.0.0 - Native Salesforce Apex C11 Runtime
Type "help", "clear", or "exit" to quit.

>>> Integer x = 10;
>>> Integer y = 20;
>>> x + y
30
>>> Account acc = new Account(Name = 'Acme Corp', Industry = 'Manufacturing');
>>> insert acc;
>>> [SELECT Id, Name FROM Account WHERE Industry = 'Manufacturing']
[Account: {Id=001000000000001AAA, Industry=Manufacturing, Name=Acme Corp}]
>>> exit
```

- **REPL Commands**: `help` (list commands), `clear` / `cls` (clear screen), `exit` / `quit` (terminate).
- **Multiline Blocks**: Unbalanced braces `{ ... }`, parentheses `( ... )`, or brackets `[ ... ]` automatically enter multiline continuation mode.

---

### 2. Single-Script Execution & Auto-Loading
Execute any standalone `.apex` or `.cls` file:

```bash
./build/nadir examples/enterprise_order_management.apex
```

If a script references a class that isn't pre-loaded (e.g. `new AccountService()`), Nadir automatically locates, parses, and registers `AccountService.cls` from the current working directory on demand.

---

### 3. Multi-File Modular Execution
Load multiple dependency classes into a shared interpreter session followed by a driver script:

```bash
./build/nadir examples/inheritance_test/Animal.cls \
              examples/inheritance_test/Mammal.cls \
              examples/inheritance_test/Dog.cls \
              examples/inheritance_test/main.apex
```

Classes, interfaces, and methods defined in preceding files become immediately visible to all subsequent files without extra boilerplate.

---

### 4. Embedding in Native C Applications
Embed the Nadir runtime directly into C/C++ applications via the public API:

```c
#include "eval.h"
#include "parser.h"
#include "lexer.h"

int main(void) {
    Interpreter* interp = interpreter_new();

    const char* apex_code = 
        "Account a = new Account(Name = 'Acme'); insert a;\n"
        "System.debug('Created account with ID: ' + a.Id);\n";

    Lexer lexer;
    lexer_init(&lexer, apex_code);

    Parser parser;
    parser_init(&parser, &lexer);

    ASTNode* program = parser_parse(&parser);
    if (!parser.had_error && program) {
        Value result = interpreter_run(interp, program);
    }

    interpreter_free(interp);
    return 0;
}
```

---

## Verification & Testing

Nadir is continuously validated against the Salesforce reference repository ([`trailheadapps/apex-recipes`](https://github.com/trailheadapps/apex-recipes)):

- **Suite**: 142 total files (139 Apex classes, 3 Apex triggers)
- **Pass Rate**: 142 / 142 (100.0%)

Run the automated test suite:

```bash
python test_runner.py
```

---

## License

This project is licensed under the [MIT License](LICENSE).

Copyright (c) 2026 James Ezra Seitenschlag.

---

## Disclaimer

Nadir is an independent open-source project and is not affiliated, associated, authorized, endorsed by, or in any way officially connected with Salesforce, Inc., or any of its subsidiaries or affiliates. The official Salesforce website can be found at [https://www.salesforce.com](https://www.salesforce.com).

"Salesforce", "Apex", "SOQL", and related marks are registered trademarks of Salesforce, Inc.

