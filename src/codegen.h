#ifndef CODEGEN_H
#define CODEGEN_H

#include <fstream>
#include "ASTNode.h"
#include "Messages.h"
#include "typechecker.h"

class CodeGenerator {
	public:
        /* ============ */
        /* Data Members */
        /* ============ */

        // classes from typechecker, typechecker instance
		std::map<std::string, Qclass*> classes;
		Typechecker *tc;
		std::vector<std::string> primitives = {};

		// indent for codegen
        std::string indent = "\n\n";

        // filename for outputting
        std::string filename;

        // static variable for counting temps
        int tempno = 1;

        /* ========================== */
        /* Constructors & Destructors */
        /* ========================== */

        CodeGenerator(Typechecker *tc, std::string fname) : classes(tc->classes), filename(fname), tc(tc) { };
        virtual ~CodeGenerator() { };

        /* ======= */
        /* Methods */
        /* ======= */

		// sample beginning code generation
		bool generate();
        bool generateClasses(std::ostream &output);
        bool generateMethods(std::ostream &output);
        bool generateMain(std::ostream &output);

        // helper functions for the main code generation
        bool checkPrimitive(std::string name);
        void generateTypedefs(std::ostream &output);
        void generateClassdefs(std::ostream &output);
        void generateMainAssign(std::ostream &output, AST::Node *stmt);
        void generateMainCall(std::ostream &output, AST::Node *stmt);
};

#endif