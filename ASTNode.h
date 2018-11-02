#ifndef ASTNODE_H
#define ASTNODE_H

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>

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

namespace AST {
	class ASTNode {
	  public:
		/* -~-~-~- Data Members -~-~-~- */
		Type type;
		std::map<Type, std::vector<ASTNode*> > children;
		std::vector<Type> order;

		std::string name;
		int value;

		/* -~-~-~- Constructor, Destructor -~-~-~- */
		ASTNode(Type type) : type(type) { };
		ASTNode(Type type, int intLiteral) : type(type), identInt(intLiteral) { };
		ASTNode(Type type, std::string strLiteral) : type(type), identStr(strLiteral) { };
		virtual ~ASTNode() {};

		/* -~-~-~- Inserting & Getting Data -~-~-~- */
		void insert(Type type, ASTNode *node);
		ASTNode *get(Type type);
		std::vector<ASTNode *> getSeq(Type type);

		/* -~-~-~- Main JSON method -~-~-~- */
		void json(std::ostream &out, AST_print_context &ctx);

		/* -~-~-~- JSON printing methods -~-~-~- */
		void json_child (std::ostream &out, AST_print_context &ctx, std::string field, ASTNode &child, char sep=',');
		void json_head  (std::ostream &out, AST_print_context &ctx, std::string node_kind);
		void json_close (std::ostream &out, AST_print_context &ctx);
		void json_indent(std::ostream &out, AST_print_context &ctx);
	};

	class AST_print_context {
	  public:
		int indent_;
		AST_print_context() : indent_{0} {};
		void indent() { ++indent_; }
		void dedent() { --indent_; }
	};
}

#endif
