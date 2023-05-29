# Compiler Project

This is a compiler project developed by Itzik Rahamim, Gil ben hamo and Yovel aloni as part of the "Compilers" course at SCE College.

## Description

This project is a compiler implementation for a simple procedural language. The compiler consists of three main components:

1. Lexical Analysis: This component uses lex tools to tokenize the input source code and generate the corresponding tokens.

2. Semantic Analysis: This component performs semantic checks on the tokenized code to ensure its correctness according to the language's rules and specifications.

3. Three Address Code Generation: This component generates three-address code, an intermediate representation of the source code that can be further processed or executed.

## Language Description

This section provides a detailed description of the syntax and semantics of the simple procedural language supported by this compiler.

### Syntax

The syntax of the language is defined using context-free grammar rules. Here is an overview of the main syntax elements:

1. Variables:
   - Variable declarations start with the keyword `var` followed by the variable name and optional initialization.
   - Supported variable types are `int`, `char`, `bool`, and `real`.
   - Multiple variables can be declared on the same line, separated by commas.

2. Control Flow:
   - Conditional statements are supported using the `if` and `else` keywords.
   - Looping is supported using the `while` and `for` keywords.

3. Expressions:
   - Arithmetic operators (`+`, `-`, `*`, `/`) can be used for mathematical operations.
   - Relational operators (`<`, `>`, `<=`, `>=`, `==`, `!=`) compare values and return boolean results.
   - Logical operators (`&&`, `||`, `!`) perform logical operations on boolean values.

4. Functions:
   - Function definitions start with the keyword `void`, followed by the function name, parameters (if any), and a block of code.
   - Functions can be called by using the function name followed by parentheses and arguments (if any).

### Semantics

The semantics of the language define the meaning and behavior of the various language constructs. Here are the key semantic rules:

1. Variables:
   - Variables must be declared before they can be used.
   - Variables have a specific type that determines the kind of data they can store.

2. Control Flow:
   - Conditional statements execute blocks of code based on the evaluation of a condition.
   - Looping statements repeatedly execute a block of code while a condition is true.

3. Expressions:
   - Expressions can involve variables, literals, and operators.
   - Arithmetic expressions perform mathematical calculations.
   - Relational expressions compare values and return boolean results.
   - Logical expressions combine boolean values using logical operators.

4. Functions:
   - Functions encapsulate reusable blocks of code.
   - Functions can have parameters that act as placeholders for values passed during function calls.
   - Functions can return values using the `return` keyword.

Please note that this is just a brief overview of the language's syntax and semantics.

## Contact Information

- Name: Itzik Rahamim
- Email: eizikr@icloud.com
- LinkedIn: [itzik-rahamim-developer](https://www.linkedin.com/in/itzik-rahamim-developer/)
