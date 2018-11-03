#ifndef ASTNODE_H
#define ASTNODE_H

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include "ASTEnum.h"

namespace AST {
	class AST_print_context {
	  public:
		int indent_;
		AST_print_context() : indent_{0} {};
		void indent() { ++indent_; }
		void dedent() { --indent_; }
	};

	class ASTNode {
	  public:
		/* -~-~-~- Data Members -~-~-~- */
		Type type;
		std::map<Type, std::vector<ASTNode*> > children;
		std::vector<Type> order;

		std::string identStr;
		int identInt;

		/* -~-~-~- Constructor, Destructor -~-~-~- */
		ASTNode(Type type) : type(type) { };
		ASTNode(Type type, int intLiteral) : type(type), identInt(intLiteral) { };
		ASTNode(Type type, std::string strLiteral) : type(type), identStr(strLiteral) { };
		virtual ~ASTNode() {};

		/* -~-~-~- Inserting & Getting Data -~-~-~- */
		void insert(ASTNode *node);
		ASTNode *get(Type type);
		std::vector<ASTNode *> getSeq(Type type);

		/* -~-~-~- JSON printing methods -~-~-~- */
		void jsonSeq    (std::ostream& out, AST_print_context& ctx);
		void json_indent(std::ostream& out, AST_print_context& ctx);
		void json_head  (std::string node_kind, std::ostream& out, AST_print_context& ctx);
		void json_close (std::ostream& out, AST_print_context& ctx);
		void json_child (std::string field, ASTNode& child, std::ostream& out, AST_print_context& ctx, char sep=',');

		/* -~-~-~- Main JSON method -~-~-~- */
		void json(std::ostream& out, AST_print_context& ctx);
	};
}

#endif
