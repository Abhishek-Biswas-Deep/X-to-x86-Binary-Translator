<img src="https://github.com/user-attachments/assets/2ad86f70-12b4-4500-997d-9f8c1874a9b5" alt="Dal logo" width="80"/>
<h1>Associated with Dalhousie University</h1>

### CSCI2122
### X to x86 Binary Translator

#### Overview
This project implements a binary translator that converts a simplified RISC-based 16-bit instruction set (X architecture) to x86-64 assembly code. The translator, named Xtra, reads X binary code and generates corresponding x86 assembly instructions.

#### Features

- Translates X architecture instructions to x86-64 assembly
- Handles various instruction types: 0-operand, 1-operand, 2-operand, and extended instructions
- Implements register mapping between X and x86-64 architectures
- Supports debug mode translation (std and cld instructions)
- Generates proper prologue and epilogue for translated code

#### Project Structure

- main.c: Contains the main() function and program entry point
- translator.c (suggested): Implements the translation logic
- Makefile: For building the project
- runit.sh: Script for testing the translator
- xas: X architecture assembler (provided)

#### Notes

- The translator handles files up to 65536 bytes (64KB)
- Output is in AT&T style assembly syntax

#### Feel free to contribute.
