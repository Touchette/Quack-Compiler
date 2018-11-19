# A Compiler for the Quack Language

This is a compiler written for Quack, a language developed by professor Michal Young at the University of Oregon. The lexer is written in C++ using the tool RE\Flex, the parser is written in C++ using the parser generator tool Bison, and the rest is written by hand.

## Lexer/Scanner ##
### quack.lxx ###

The lexer is written in RE\Flex: it tokenizes the Quack language appropriately for the parser to build a parse tree with. It matches patterns using regular expressions, the most complicated being string literals and comments. Other than that, it is pretty simple: regular expressions can match the patterns in Quack quite succinctly. An example regular expression for multi-line comments:

```c
   /* Multi-line comments */
[/][*]           { start(COMMENT); }
<COMMENT>[^*]*   { ; }
<COMMENT>[*][^/] { ; }
<COMMENT>[*][/]  { start(INITIAL); }
```

These regular expressions will, upon seeing `/*`, enter a so-called "scanner state" in which only the rules preceded by `<COMMENT>` are followed. When the scanner reads `[^*]*` (0 or more of anything that isn't an asterisk), it discards what it read (as a comment should do). When the scanner encounters `[*][^/]` (an asterisk followed by anything that is not /), it also throws it away, as the comment has not ended yet. Finally, upon finding `*/`, which closes a comment, the scanner returns to its initial start state with `start(INITIAL);`. 

## Parser ##
### quack.yxx, parser.cpp, names.h ###

The parser is written in Bison: it outlines the grammar of the Quack language and (hopefully) produces a parse tree that will be turned into the abstract syntax tree. It also outputs syntax errors when given programs do not conform to the grammar correctly and also notifies when a program is syntactically correct and well-structured. An example grammar rule/production for matching/parsing method calls:

```
method
    : DEF IDENT '(' formal_args ')' statement_block { }
    | DEF IDENT '(' formal_args ')' ':' IDENT statement_block { }
    ;
```

This production is entered from the following "hierarchy": program -> classes -> class -> methods -> method, where each of these is a non-terminal in the Quack grammar. Upon attempting to parse a method, this production will:
- Match a token `DEF`, which starts a method definition
- Match an identifier token `IDENT`, which is the name of the method
- Match an opening paren `'('`, which will contain the arguments to the function
- Match all the arguments to the method, the rules for which are contained in another non-terminal `formal_args`
- Match the closing paren `')'`
- Finally match the method statement `statement_block`, another non-terminal which will contain the actual code that the function executes

A similar pattern can be seen in the other option for a method call, wherein a `':'` (colon) is matched in order to serve as an initializer with another `IDENT` token.

## Abstract Syntax Tree ##

TBD

## Type Checking ##

TBD

## Code Generation ##

TBD
