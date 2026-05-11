# Compiler Construction & Execution Pipeline

## Overview
This repository contains a three-part journey into compiler construction and language processing, written in C++11. Rather than relying on parser generators like YACC or Bison, this project implements the core components of a compiler front-end and interpreter entirely from scratch. 

The repository is divided into three isolated phases, each representing a critical step in translating human-readable code into machine-executable instructions:

1. **Lexical Analysis:** Converting raw text into categorized tokens.
2. **Context-Free Grammar (CFG) Analysis:** Transforming and validating language grammars for predictive parsing.
3. **Intermediate Representation (IR) Generation:** Lowering high-level syntax into an executable instruction graph and interpreting it.

## Learning Objectives
* **Theory to Practice:** Bridged theoretical computer science concepts (Regular Expressions, Context-Free Grammars, LL(k) Parsing) with practical software engineering.
* **Algorithmic Complexity:** Implemented fixed-point iteration algorithms for set theory calculations (FIRST/FOLLOW sets) and dynamic graph generation algorithms (IR lowering).
* **Memory & Control Flow:** Managed simulated memory allocation for variables and "desugared" complex control flows (Switch cases, For loops) into fundamental Jump/Conditional-Jump instructions.

## Repository Structure
Each phase is contained within its own directory with a dedicated README, source code, and test suite to ensure isolated, stable builds.
* `/Project1-Lexer/` - Advanced Lexical Analyzer
* `/Project2-Grammar-Analyzer/` - CFG Transformation & Set Calculator
* `/Project3-IR-Compiler/` - Recursive Descent Parser & IR Interpreter