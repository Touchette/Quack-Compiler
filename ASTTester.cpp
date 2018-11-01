#include "ASTNode.h"
#include <iterator>
#include <iostream>
#include <typeinfo>

void printTest(std::vector<AST::ASTNode *> vec) {

	std::vector<AST::ASTNode *>::iterator it;

	for (it = vec.begin(); it != vec.end(); ++it) {
		printf("%s\n", (*it)->identStr.c_str());
		printf("%d\n", (*it)->type);
	}
}

int main() {
    AST::ASTNode* root = new AST::ASTNode(CLASSES);

	/* insert variable1 */
	root->insert(new AST::ASTNode(IDENT, "variable"));

	/* insert variable2 */
	root->insert(new AST::ASTNode(IDENT, "variable2"));
	std::vector<AST::ASTNode *> vec = root->getSeq(IDENT);

	printTest(vec);
}
