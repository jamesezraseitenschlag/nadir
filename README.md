# Nadir - Native Salesforce Apex Runtime Engine

Nadir is a lightweight, high-performance, cross-platform interpreter and runtime engine for the Salesforce Apex language, implemented in ISO C11 with zero external dependencies.

---

## Architecture Overview

The runtime is designed with modularity, deterministic memory management, and cross-platform compatibility as core principles:

- **Lexer & Tokenizer**: Case-insensitive UTF-8 stream scanner supporting Apex keywords, SOQL operators, annotations, and literals.
- **Recursive-Descent Parser**: Grammar parser structured into dedicated subsystems for declarations, statements, expressions, and inline SOQL.
- **Abstract Syntax Tree (AST)**: Arena-managed syntax tree nodes representing OOP constructs, control flow, collections, and database operations.
- **Interpreter & Evaluator**: Modular tree-walking execution engine with scoped environments, call stack unwinding, and runtime error reporting.
- **SObject & Mock Database Engine**: In-memory relational database providing SObject storage, 15/18-character Salesforce ID generation, SOQL querying, savepoints, rollback, and DML transaction tracking.
- **Standard Library Subsystem**: Built-in mock implementations for `System`, `Math`, `String`, `JSON`, `Database`, `Test`, `Limits`, `UserInfo`, and `Schema` namespaces.

---

## Project Structure

```text
nadir/
├── CMakeLists.txt             # Cross-platform CMake build configuration
├── README.md                  # Project documentation
├── test_runner.py             # Automated test suite runner
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

### Prerequisites
- A standard C11 compiler (`gcc`, `clang`, `msvc`, or `zig cc`)
- `CMake` (version 3.15 or newer)

### Standard Native Build (Linux, macOS, Windows)

```bash
cmake -B build
cmake --build build --config Release
```

The resulting executable will be available at:
- **Linux / macOS**: `build/nadir`
- **Windows**: `build/Release/nadir.exe` (or `build/nadir.exe`)

---

## Cross-Compilation

Nadir relies strictly on standard C11 and portable POSIX/C runtime interfaces, enabling seamless cross-compilation:

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

## Usage

### Interactive REPL Mode
Running the executable without parameters starts the interactive REPL:

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

### Script Execution
Execute an Apex source file directly by passing its path:

```bash
./build/nadir examples/enterprise_order_management.apex
```

---

## Verification & Testing

Nadir is continuously validated against the Salesforce reference repository (`apex-recipes`):

- **Suite**: 142 total files (139 Apex classes, 3 Apex triggers)
- **Pass Rate**: 142 / 142 (100.0%)

Run the automated test suite:

```bash
python test_runner.py
```

---

## License

This project is open-source under the MIT License.
