# Arion Compiler - Formal Language and Automata Theory

## IF2224 - Formal Language and Automata Theory

This program is made for IF2224 Formal Language and Automata Theory project submission at ITB Informatics Engineering.

## Core Features

Arion is a compiler-interpreter for a Pascal-like language, covering the full pipeline from source text to program execution:

- **Lexical Analysis** : Tokenizes raw source code into a stream of meaningful tokens.
- **Syntax Analysis** : Builds a parse tree from the token stream using a recursive-descent parser based on the Arion grammar.
- **Semantic Analysis** : Produces a Decorated AST with type checking, scope resolution, and symbol tables (`tab`, `btab`, `atab`).
- **Intermediate Code Generation** : Flattens the Decorated AST into linear stack-machine instructions (`INT`, `LIT`, `LOD`, `STO`, `CAL`, `JMP`, `JPC`, `OPR`, `RET`).
- **Interpreter (Stack Machine)** : Executes the intermediate code on a stack-based virtual machine with call frames and static/dynamic links, producing real program output.
- **Runtime Safety** : Detects stack overflow/underflow, out-of-bounds access, invalid jumps, and division/modulo by zero without crashing.

## Dirty Chocolate - DCL

### Author

Made Branenda Jordhy<br>
13524026<br>
[GitHub Account](https://github.com/ethj0r)<br>

Muhammad Nur Majiid<br>
13524028<br>
[GitHub Account](https://github.com/MAJIIDMN)<br>

Jason Edward Salim<br>
13524034<br>
[GitHub Account](https://github.com/jsndwrd)<br>

Athilla Zaidan Zidna Fann<br>
13524068<br>
[GitHub Account](https://github.com/AthillaZaidan)<br>

## Directory

1. `src`: source code.
2. `doc`: project documentation.
3. `test`: testcases and results.
4. `bin`: executables.
5. `README.md`: project outline.

## Requirements

- `g++` (GNU C++ Compiler) for C++ source code compilation.
- `make` for build process automation via Makefile.

## Instructions

### Installation

Make sure `g++` and `make` are installed on your machine.

### Build

```bash
make
```

or explicitly:

```bash
make build
```

Compilation is saved in `bin/lexer` (Linux) or `bin/lexer.exe` (Windows).

---

### How to Run

```bash
make run
```

### Remove Old Compilation

```bash
make clean
```

## Task Contribution

### Milestone 1

| NIM      | Name                      | Percentage |
| -------- | ------------------------- | ---------- |
| 13524026 | Made Branenda Jordhy      | 25%        |
| 13524028 | Muhammad Nur Majiid       | 25%        |
| 13524034 | Jason Edward Salim        | 25%        |
| 13524068 | Athilla Zaidan Zidna Fann | 25%        |

### Milestone 2

| NIM      | Name                      | Percentage |
| -------- | ------------------------- | ---------- |
| 13524026 | Made Branenda Jordhy      | 25%        |
| 13524028 | Muhammad Nur Majiid       | 25%        |
| 13524034 | Jason Edward Salim        | 25%        |
| 13524068 | Athilla Zaidan Zidna Fann | 25%        |

### Milestone 3

| NIM      | Name                      | Percentage |
| -------- | ------------------------- | ---------- |
| 13524026 | Made Branenda Jordhy      | 25%        |
| 13524028 | Muhammad Nur Majiid       | 25%        |
| 13524034 | Jason Edward Salim        | 25%        |
| 13524068 | Athilla Zaidan Zidna Fann | 25%        |

### Milestone 4

| NIM      | Name                      | Percentage |
| -------- | ------------------------- | ---------- |
| 13524026 | Made Branenda Jordhy      | 25%        |
| 13524028 | Muhammad Nur Majiid       | 25%        |
| 13524034 | Jason Edward Salim        | 25%        |
| 13524068 | Athilla Zaidan Zidna Fann | 25%        |