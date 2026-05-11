# Phase 2: Context-Free Grammar (CFG) Analyzer

## Overview
Before a compiler can parse code, the language's grammar must be unambiguous and structured correctly. This project automates the calculation of predictive parsing sets and performs algorithmic transformations on arbitrary grammars to make them suitable for Top-Down (LL) parsing.

## Features Implemented
* **Set Calculation:** Implemented iterative, fixed-point algorithms to automatically compute **Nullable**, **FIRST**, and **FOLLOW** sets for any given grammar.
* **Left Factoring:** Developed a transformation algorithm to detect rules sharing common prefixes and dynamically generate new non-terminals to factor them out, removing parser ambiguity.
* **Left Recursion Elimination:** Implemented a complex nested-loop algorithm to detect and eliminate both *direct* and *indirect* left recursion, ensuring recursive descent parsers do not fall into infinite loops.

## Technical Concepts
* LL(1) Predictive Parsing Theory
* Set Theory & Fixed-Point Iteration
* Abstract Syntax Tree (AST) / Grammar Transformations
* Lexicographic Sorting & Data Structure Management
