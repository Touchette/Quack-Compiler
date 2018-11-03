// Andrew Letz
// Last Modified: 10/31/2018
// Acknowledgements: Dr. Michal Young for starter code

#ifndef ASTENUM_H
#define ASTENUM_H

#include <set>

enum Type {
        /* -~-~-~-~- Begin NonTerminal Types -~-~-~-~- */
        PROGRAM, BLOCK,
        CLASSES, CLASS,
        METHODS, METHOD, CONSTRUCTOR, CALL,
        FORMAL_ARGS, FORMAL_ARGS_EXTRA, ACTUAL_ARGS_EXTRA,
        ASSIGN, RETURN, BINOP, AND, OR, DOT, LOAD, IF, WHILE,
        L_EXPR, IDENT, INTCONST, STRCONST,
        TYPECASE, TYPE_ALTERNATIVE, TYPE_ALTERNATIVE_EXTRA,
        LEAF,
        /* -~-~-~-~- Begin Identifier Types -~-~-~-~- */
        NAME
};

static const char * TypeString[] = {   
        /* -~-~-~-~- Begin NonTerminal Types -~-~-~-~- */
        "Program", "block_",
        "classes_", "class",
        "methods_", "Method", "constructor", "call",
        "formal_args", "formal_args_extra_", "actual_args_extra",
        "assign", "return", "binop_", "and_", "or_", "dot_", "load", "if", "while",
        "l_expr_", "ident", "intConst", "strConst",
        "typecase", "type_alternative",  "type_alternative_extra_",
        "leaf",
        /* -~-~-~-~- Begin Identifier Types -~-~-~-~- */
        "name_"
};

extern std::set<Type> HeadTypes;

extern std::set<Type> SeqTypes;
extern bool isSeqType(Type type);

extern std::string typeString(Type type);

#endif