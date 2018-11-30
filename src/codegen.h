#ifndef CODEGEN_H
#define CODEGEN_H

#include <fstream>
#include "ASTNode.h"
#include "Messages.h"
#include "typechecker.h"
#include <list>

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

        // variable for counting temps, variable for printing arguments correctly
        int tempno = 1;
        int i = 0;

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
        bool generateMain(std::ostream &output);

        // helper functions for the main code generation
        bool checkPrimitive(std::string name);
        void generateTypedefs(std::ostream &output);
        // helper functions for generateClasses
        void generateForwardDecls(std::ostream &output);
		void generateStructs(std::ostream &output);
		void generateExterns(std::ostream &output);
		void generateMethods(std::ostream &output);
		void generateSingletons(std::ostream &output);
		// helper functions for generateMain
        void generateMainCall(std::ostream &output, AST::Node *stmt);
        // helper function for generating statements
        std::string generateStatement(std::ostream &output, AST::Node *stmt, Qmethod *whichMethod, std::string whichClass="main");
};

#endif