# Phase 1: Lexical Analysis

## Overview
This phase focuses on the first step of compilation: lexical analysis. The objective was to extend a base lexical analyzer to recognize and process complex, custom token types using theoretical regular expressions translated into C++ logic.

## Features Implemented
* **Real Numbers:** Tokenization of floating-point numbers (`REALNUM`) requiring strict adherence to digit and decimal placement.
* **Base-8 (Octal) Numbers:** Recognition of octal configurations (`BASE08NUM`) utilizing specific prefix/suffix markers and constrained digit sets (0-7).
* **Base-16 (Hexadecimal) Numbers:** Recognition of hex values (`BASE16NUM`) incorporating both numeric and alphabetic (A-F) character constraints.
* **State Management:** Safely peeking, consuming, and un-getting characters from an input buffer to resolve token ambiguities.

## Technical Concepts
* Deterministic Finite Automata (DFAs)
* Regular Expressions
* Input Buffering & Stream Processing
