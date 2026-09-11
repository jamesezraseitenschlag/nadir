# Nadir - Native Salesforce Apex Runtime Engine

Nadir is a lightweight, ultra-high-performance, cross-platform interpreter, relational database mock, and API daemon for the Salesforce Apex programming language and Lightning Platform ecosystem. Implemented in ISO C11 with zero external runtime dependencies, Nadir delivers near-instant execution for local testing, CI/CD pipelines, and Lightning Web Component (LWC/LWR) local development.

---

## Key Features

- **Standard Apex Conformance**: Full support for object-oriented programming (classes, interfaces, inheritance, polymorphism, abstract classes), triggers, control flow, exception handling (`try-catch-finally`), and collections (`List<T>`, `Set<T>`, `Map<K, V>`).
- **In-Memory Relational Engine & SOQL**: Built-in mock database supporting SObjects, monotonic 15/18-character Salesforce ID generation with checksums, DML operations (`insert`, `update`, `delete`, `upsert`), savepoints/rollback, and inline SOQL filtering with projections and aggregates.
- **Local Salesforce API Daemon & Gateway**: Embedded HTTP/JSON daemon (`--daemon [port]`) simulating Salesforce Platform endpoints for local Lightning Web Components (LWC), Lightning Web Runtime (LWR), and Vite/Node dev servers without cloud latency:
  - **`@AuraEnabled` Gateway** (`/aura`, `/webruntime/api`): Dispatches imperative and wired Apex controller calls with parameter deserialization and typed DTO/SObject responses.
  - **Lightning Data Service (LDS) UI-API Mock** (`/services/data/v58.0/ui-api/*`): Full support for `@wire(getRecord)` and `@wire(getObjectInfo)`.
  - **Salesforce REST API Subsystem**: SOQL REST query (`/services/data/v58.0/query`), SObject CRUD (`/services/data/v58.0/sobjects/*`), Tooling API (`/services/data/v58.0/tooling/executeAnonymous`), Limits API, and OAuth2 UserInfo.
- **Cache-Local High-Performance Subsystem**: Open-addressing hash tables powered by MurmurHash3 and xxHash algorithms for zero-overhead symbol lookup, environment binding, and field resolution.
- **Developer Tooling & Ecosystem Bridges**:
  - **Node.js / NPM Package** (`npm/`): Programmatic Node.js bridge to manage and interact with local Nadir runtime instances.
  - **VS Code Extension** (`vscode-extension/`): Direct editor integration for syntax highlighting, instant script execution, and local daemon lifecycle management.
- **Zero Runtime Dependencies**: Self-contained C11 codebase compilable with GCC, Clang, MSVC, Zig CC, and Emscripten (WebAssembly).

---

## Dependencies & Prerequisites

Nadir is deliberately designed without third-party runtime library requirements. All required development and testing tools are linked below:

### Core Runtime & Build Dependencies
- **C Standard Library**: Standard C11 runtime ([ISO/IEC 9899:2011](https://www.iso.org/standard/57853.html)) with POSIX math library ([`libm`](https://man7.org/linux/man-pages/man3/math.h.0p.html)).
- **Build System**: [CMake](https://cmake.org/) (version 3.15 or newer).
- **C Compiler**: Any standard-compliant C11 compiler:
  - [GCC](https://gcc.gnu.org/) (GNU Compiler Collection, version 7.0+)
  - [Clang / LLVM](https://clang.llvm.org/) (version 6.0+)
  - [Microsoft Visual C++ (MSVC)](https://visualstudio.microsoft.com/) (Visual Studio 2019 or newer)
  - [Zig](https://ziglang.org/) (`zig cc` toolchain for instant cross-compilation)
  - [Apple Clang](https://developer.apple.com/xcode/) (Xcode 11 or newer)

### Test Harness & Validation Datasets
- **Test Automation**: [Python 3](https://www.python.org/) (version 3.7+) and [Node.js](https://nodejs.org/) (version 16+).
- **Benchmark & Reference Apps**:
  - [Salesforce Trailhead Apex Recipes](https://github.com/trailheadapps/apex-recipes) for language conformance testing.
  - [Salesforce Trailhead Dreamhouse LWC](https://github.com/trailheadapps/dreamhouse-lwc) for enterprise LWC and LDS API validation.
- **Language Reference**: [Salesforce Apex Developer Guide](https://developer.salesforce.com/docs/atlas.en-us.apexcode.meta/apexcode/).

### Grammar Specifications & Origins
Reference ANTLR4 grammars in `grammars/` were sourced and adapted from:
- [ANTLR grammars-v4 Community Repository](https://github.com/antlr/grammars-v4) for foundational grammar patterns.
- [Salesforce apex-jorje & PMD Apex Parser](https://github.com/pmd/pmd/tree/master/pmd-apex) for Apex-specific AST structure and keyword mappings.

---

## Architecture Overview

The runtime is structured into modular, decoupled subsystems:

- **Lexer & Tokenizer** ([`src/lexer.c`](file:///C:/Users/James/ARFS/src/lexer.c)): Case-insensitive UTF-8 stream scanner supporting Apex keywords, SOQL operators, annotations, and literals.
- **Recursive-Descent Parser** ([`src/parser.c`](file:///C:/Users/James/ARFS/src/parser.c)): Modular grammar parser with dedicated modules for declarations ([`src/parser_decl.c`](file:///C:/Users/James/ARFS/src/parser_decl.c)), statements ([`src/parser_stmt.c`](file:///C:/Users/James/ARFS/src/parser_stmt.c)), expressions ([`src/parser_expr.c`](file:///C:/Users/James/ARFS/src/parser_expr.c)), and inline SOQL ([`src/parser_soql.c`](file:///C:/Users/James/ARFS/src/parser_soql.c)).
- **Abstract Syntax Tree** ([`src/ast.c`](file:///C:/Users/James/ARFS/src/ast.c)): Arena-managed syntax tree nodes representing OOP constructs, control flow, collections, and database operations.
- **Interpreter & Evaluator** ([`src/eval.c`](file:///C:/Users/James/ARFS/src/eval.c)): Modular tree-walking execution engine with scoped environments, call stack unwinding, and runtime error reporting.
- **Operator Engine** ([`src/eval_op.c`](file:///C:/Users/James/ARFS/src/eval_op.c)): Fast-path arithmetic, relational, logical, and assignment operations with automatic type coercion.
- **High-Performance Hashing Engine** ([`src/nadir_hash.c`](file:///C:/Users/James/ARFS/src/nadir_hash.c)): MurmurHash3 and xxHash open-addressing lookup tables with cache-friendly contiguous memory layout.
- **Standard Library Subsystem** ([`src/eval_sys.c`](file:///C:/Users/James/ARFS/src/eval_sys.c)): Built-in mock implementations for `System`, `Math`, `String`, `JSON`, `Database`, `Test`, `Limits`, `UserInfo`, and `Schema` namespaces.
- **Method & Dispatch Engine** ([`src/eval_call.c`](file:///C:/Users/James/ARFS/src/eval_call.c)): Dynamic dispatch for static class methods, instance methods, and collection manipulation.
- **SObject & Mock Database Engine** ([`src/sobject.c`](file:///C:/Users/James/ARFS/src/sobject.c)): In-memory relational storage providing SObject records, monotonic global 15/18-character Salesforce ID generation, SOQL querying, savepoints, rollback, and bulk DML transaction tracking.
- **Local API Daemon & Web Runtime Gateway** ([`src/nadir_daemon.c`](file:///C:/Users/James/ARFS/src/nadir_daemon.c), [`src/mongoose.c`](file:///C:/Users/James/ARFS/src/mongoose.c)): High-throughput embedded HTTP daemon providing Salesforce-compatible `@AuraEnabled`, LDS UI-API, SOQL REST, and Tooling API endpoints.

---

## Project Structure

```text
nadir/
├── CMakeLists.txt             # Cross-platform CMake build configuration
├── LICENSE                    # MIT License
├── README.md                  # Project documentation
├── test_runner.py             # Main language regression test runner (142 tests)
│
├── .github/
│   └── workflows/
│       └── build.yml          # Multi-platform CI/CD and packaging workflow
├── .gitlab-ci.yml             # GitLab CI pipeline configuration
│
├── include/                   # C11 Header declarations
│   ├── ast.h                  # AST node types and constructors
│   ├── common.h               # Portable platform definitions and string utilities
│   ├── env.h                  # Lexical environment and symbol table
│   ├── eval.h                 # Public interpreter API
│   ├── eval_internal.h        # Internal evaluation and dispatch prototypes
│   ├── lexer.h                # Lexer interface and state
│   ├── mongoose.h             # Embedded HTTP server interface
│   ├── nadir_daemon.h         # Local API daemon & gateway declarations
│   ├── nadir_hash.h           # MurmurHash3 & xxHash hashing interfaces
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
│   ├── main.c                 # CLI executable entry point, REPL shell, and daemon launcher
│   ├── metadata.c             # Salesforce metadata & schema loader
│   ├── mongoose.c             # Embedded networking engine
│   ├── nadir_daemon.c         # Local Salesforce HTTP API Daemon & UI-API Mock
│   ├── nadir_hash.c           # High-speed hashing implementation
│   ├── parser.c               # Parser initialization and token navigation
│   ├── parser_decl.c          # Classes, interfaces, triggers, and declarations
│   ├── parser_expr.c          # Expression precedence climbing and literals
│   ├── parser_soql.c          # Inline SOQL query parsing
│   ├── parser_stmt.c          # Control flow, loops, and statement parsing
│   ├── sobject.c              # SObject records, DML, and mock SOQL engine
│   ├── token.c                # Token diagnostics and string conversion
│   └── value.c                # Value constructors, collections, and comparisons
│
├── npm/                       # Node.js & NPM Bridge Package
│   ├── package.json           # NPM package configuration
│   └── lib/index.js           # Programmatic Node.js daemon and runner API
│
├── vscode-extension/          # Visual Studio Code Extension
│   ├── package.json           # Extension manifest and command contributions
│   └── src/extension.js       # VS Code runtime integration
│
├── qa_test_suite/             # Comprehensive 5-Pillar QA test framework
│   └── run_all.py             # Automated QA test runner
│
├── lwc_test_suite/            # Advanced Aura & LWC compatibility tests
│   └── test_aura_lwc_compat.js# HTTP API compatibility validation script
│
├── examples/                  # Apex code examples & enterprise patterns
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

Nadir provides multiple execution modes tailored for standalone scripts, interactive debugging, native C embedding, and local Salesforce web development:

### 1. Local Salesforce API Daemon & Gateway (`--daemon`)

Start the embedded API daemon to serve local Lightning Web Components, LWR apps, and front-end dev servers without requiring a cloud org:

```bash
./build/nadir --daemon 8088
```

#### Supported Endpoints & Protocols:
| Endpoint | Method | Description |
| :--- | :---: | :--- |
| `/status` | `GET` | Daemon health check, uptime, and engine status |
| `/aura` | `POST` | `@AuraEnabled` Gateway (Imperative, wire, & legacy Aura action payloads) |
| `/webruntime/api` | `POST` | LWR (Lightning Web Runtime) Gateway endpoint |
| `/services/data/v58.0/ui-api/records/:id` | `GET`, `POST`, `DELETE` | Lightning Data Service (LDS) `@wire(getRecord)` mock |
| `/services/data/v58.0/ui-api/object-info/:name` | `GET` | LDS `@wire(getObjectInfo)` mock |
| `/services/data/v58.0/query?q=...` | `GET` | SOQL REST Query endpoint |
| `/services/data/v58.0/sobjects/:name` | `GET`, `POST` | SObject REST CRUD endpoint |
| `/services/data/v58.0/sobjects/:name/:id` | `GET`, `PATCH`, `DELETE` | SObject Record REST CRUD endpoint |
| `/services/data/v58.0/tooling/executeAnonymous` | `GET`, `POST` | Tooling API Anonymous Apex execution |
| `/services/data/v58.0/limits` | `GET` | Salesforce Governor Limits API mock |
| `/services/data` | `GET` | Salesforce API Versions API |
| `/services/oauth2/userinfo` | `GET` | Salesforce OAuth2 UserInfo mock |

#### Example: Calling `@AuraEnabled` Apex Controller from Frontend
```bash
curl -X POST http://localhost:8088/aura \
  -H "Content-Type: application/json" \
  -d '{
    "className": "PropertyController",
    "methodName": "getPagedPropertyList",
    "params": { "searchKey": "Boston", "maxPrice": 1500000 }
  }'
```

---

### 2. Interactive REPL Mode
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

### 3. Single-Script Execution & Auto-Loading
Execute any standalone `.apex` or `.cls` file:

```bash
./build/nadir examples/enterprise_order_management.apex
```

If a script references a class that isn't pre-loaded (e.g. `new AccountService()`), Nadir automatically locates, parses, and registers `AccountService.cls` from the current working directory on demand.

---

### 4. Multi-File Modular Execution
Load multiple dependency classes into a shared interpreter session followed by a driver script:

```bash
./build/nadir examples/inheritance_test/Animal.cls \
              examples/inheritance_test/Mammal.cls \
              examples/inheritance_test/Dog.cls \
              examples/inheritance_test/main.apex
```

Classes, interfaces, and methods defined in preceding files become immediately visible to all subsequent files without extra boilerplate.

---

### 5. Node.js & NPM Bridge
Nadir includes a programmatic Node.js SDK for seamless integration into JavaScript/TypeScript build tools and test runners:

```javascript
const { NadirRuntime } = require('./npm/lib/index.js');

const nadir = new NadirRuntime({ port: 8088 });
await nadir.start();

const result = await nadir.executeAnonymous(`
    Account a = new Account(Name = 'Universal Containers');
    insert a;
    return a.Id;
`);

console.log('Created Account ID:', result);
await nadir.stop();
```

---

### 6. Embedding in Native C Applications
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

## Verification & Testing Matrix

Nadir is validated across four test suites covering language syntax, semantic runtime safety, enterprise LWC compatibility, and real-world reference applications:

| Test Suite | Focus Area | Tests | Result |
| :--- | :--- | :---: | :---: |
| **Language Conformance** (`test_runner.py`) | Apex Recipes reference test suite (139 classes, 3 triggers) | 142 / 142 | **100.0% PASS** |
| **Comprehensive QA Framework** (`qa_test_suite/run_all.py`) | 5-pillar verification (Lexer, Types, Runtime, SObjects, Fuzzer) | 15 / 15 | **100.0% PASS** |
| **Aura & LWC Compatibility** (`lwc_test_suite/test_aura_lwc_compat.js`) | Local daemon, `@AuraEnabled`, LDS UI-API, SOQL REST, CORS | 19 / 19 | **100.0% PASS** |
| **Dreamhouse Enterprise E2E** (`test_repos/deploy_and_test_real_world_app.js`) | Real-world Dreamhouse LWC app with 1,000+ records bulk ingestion | 13 / 13 | **100.0% PASS** |
| **Total Comprehensive Verification** | **Full System Conformance** | **189 / 189** | **100.0% PASS** |

To execute the test suites:

```bash
# Core language regression suite
python test_runner.py

# 5-Pillar QA test suite
python qa_test_suite/run_all.py

# Aura & LWC compatibility suite (requires daemon running)
node lwc_test_suite/test_aura_lwc_compat.js
```

---

## License

This project is licensed under the [MIT License](LICENSE).

Copyright (c) 2026 James Ezra Seitenschlag.

---

## Disclaimer

Nadir is an independent open-source project and is not affiliated, associated, authorized, endorsed by, or in any way officially connected with Salesforce, Inc., or any of its subsidiaries or affiliates. The official Salesforce website can be found at [https://www.salesforce.com](https://www.salesforce.com).

"Salesforce", "Apex", "SOQL", and related marks are registered trademarks of Salesforce, Inc.


