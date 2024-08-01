# Assembler for Hypothetical Processor

## Project Overview

This project involves creating an assembler in C for an imaginary assembly language. The assembler translates human-readable code into machine code for a hypothetical processor. This project was undertaken to strengthen programming skills and problem-solving abilities.

## Features

- **Parsing Algorithms**: Developed to interpret assembly instructions.
- **Symbol Table Management**: Efficient handling of labels and variables.
- **Machine Code Generation**: Creation of executable machine code from assembly instructions.

## File Structure

- **compile.c**: Contains functions for compiling the assembly code.
- **define.h**: Header file defining constants and macros.
- **first_iteration.c**: Updates the symbol table and creates machine code not dependent on the symbol table.
- **linked_list.c/h**: Implementation of linked lists used in the project.
- **main.c**: Entry point for the assembler.
- **makefile**: Script for building the project.
- **pre_processor.c/h**: Preprocessor logic to handle macros.
- **second_iteration.c**: Creates the final machine code.
- **symbol_table.c/h**: Manages the symbol table for labels and variables.
- **syntax.c/h**: Syntax checking and validation functions.

## Usage

1. **Compiling the Assembler**:
    ```bash
    make
    ```

2. **Running the Assembler**:
    ```bash
    ./assembler input_file.as
    ```

3. **Output**:
    The assembler will generate an output file with the machine code corresponding to the input assembly file.


