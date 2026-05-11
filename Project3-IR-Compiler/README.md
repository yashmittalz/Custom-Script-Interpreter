# Phase 3: IR Generator & Interpreter

## Overview
The final phase ties parsing and execution together. This project implements a strict Recursive Descent Parser for a custom programming language. Instead of compiling to assembly, the parser lowers the code into an Intermediate Representation (IR)—a linked-list/graph of instruction nodes—which is then executed by a custom runtime interpreter.

## Features Implemented
* **Recursive Descent Parsing:** Built a parser from scratch that verifies syntax while simultaneously generating instruction nodes.
* **Memory Management:** Implemented a symbol table tracking system that maps string-based variable names to integer-based memory addresses dynamically.
* **Control Flow Desugaring:** 
  * Lowered standard operations (`ASSIGN`, `INPUT`, `OUTPUT`).
  * Translated `IF` and `WHILE` statements into graph-based Conditional Jumps (`CJMP`) and Unconditional Jumps (`JMP`).
  * "Desugared" `FOR` loops into `WHILE` loop equivalents.
  * Translated `SWITCH` statements into chained `CJMP` blocks utilizing `NOTEQUAL` logic to handle case fall-through and execution.
* **Runtime Execution:** The compiled IR graph is traversed and executed, simulating a real processor reading memory and executing jumps.

## Technical Concepts
* Syntax-Directed Translation
* Intermediate Representation (IR) Graph Building
* Control Flow Desugaring
* Memory Mapping & Runtime Interpretation
