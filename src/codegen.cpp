#include "codegen.h"

bool CodeGenerator::generate() {
	primitives.push_back("String");
	primitives.push_back("Boolean");
	primitives.push_back("Int");
	primitives.push_back("Obj");
	primitives.push_back("Nothing");

	std::ofstream fout(filename);

	// include the built-in functions and classes provided by Professor Young
	fout << "#include \"src/Builtins.h\"" << indent;

    bool classesGenerated = this->generateClasses(fout);
    if (!classesGenerated) {
    	report::error("code for classes could not be generated!", CODEGENERATION);
        report::dynamicBail();
    } else if (report::ok()) {
        report::gnote("code for classes successfully generated.", CODEGENERATION);
    }

	bool mainGenerated = this->generateMain(fout);
    if (!mainGenerated) {
    	report::error("code for main method could not be generated!", CODEGENERATION);
        report::dynamicBail();
    } else if (report::ok()) {
        report::gnote("code for main method successfully generated.", CODEGENERATION);
    }

	return true;
}

bool CodeGenerator::checkPrimitive(std::string name) {
	if (std::find(primitives.begin(), primitives.end(), name) != primitives.end()) {
		return true;
	} 
	return false;
}

bool CodeGenerator::generateClasses(std::ostream &output) {
	// print out the typedef'd structs for every class
	generateTypedefs(output);

	// output the forward declarations of "the_class_CLASSNAME_struct"s
	generateForwardDecls(output);

	// output the struct obj_CLASSNAME_struct with fields and clazz pointer,
	// output the struct class_CLASSNAME_struct with methods
	generateStructs(output);

	// idk what this is but it's in michal young's output so here it is
	generateExterns(output);

	// output the methods for each class
	generateMethods(output);

	// output the singleton classes for each class
	generateSingletons(output);

	// final whitespace!
	output << indent;

	return true;
}

void CodeGenerator::generateTypedefs(std::ostream &output) {
	output << "// -~-~-~-~- Typedefs Begin -~-~-~-~-" << indent;
	for (auto qclass : this->classes) {
		auto currentClass = qclass.second;
		std::string name = currentClass->name;
		Qmethod *constructor = currentClass->constructor;
		if (checkPrimitive(name)) {
			continue;
		} 

		// output the definitions for this class
		output << "// Class " << name << "'s typedefs" << std::endl;
		output << "struct class_" << name << "_struct;" << std::endl;

		output << "typedef struct class_" << name << 
		"_struct* class_"<< name << ";" << std::endl;

		output << std::endl;
	}
	output << "// -~-~-~-~- Typedefs End -~-~-~-~-" << indent;
}

void CodeGenerator::generateForwardDecls(std::ostream &output) {
	output << "// -~-~-~-~- Forward Declarations Begin -~-~-~-~-" << indent;
	for (auto qclass : this->classes) {
		auto currentClass = qclass.second;
		std::string name = currentClass->name;
		Qmethod *constructor = currentClass->constructor;
		if (checkPrimitive(name)) {
			continue;
		} 
		output << "// Class " << name << "'s forward declarations" << std::endl;
		output << "struct class_" << name << "_struct the_class_" << name << "_struct;" << indent;
	}
	output << "// -~-~-~-~- Forward Declarations End -~-~-~-~-" << indent;
}

void CodeGenerator::generateStructs(std::ostream &output) {
	output << "// -~-~-~-~- Structs Begin -~-~-~-~-" << indent;
	for (auto qclass : this->classes) {
		auto currentClass = qclass.second;
		std::string name = currentClass->name;
		Qmethod *constructor = currentClass->constructor;
		if (checkPrimitive(name)) {
			continue;
		}

		// output the struct obj_CLASSNAME_struct with fields and clazz pointer
		output << "typedef struct obj_" << name << "_struct {" << std::endl;
		output << "\tclass_" << name << " clazz;" << std::endl;

		for (auto field : currentClass->instanceVarType) {
			if (field.first != "this") {
				output << "\tobj_" << field.second << " " << field.first << ";" << std::endl;
			} 
		}
		output << "} * obj_" << name << ";" << indent;

		// output the "struct class_CLASSNAME_struct"
		output << "struct class_" << name << "_struct {" << std::endl;
		output << "\t// Method Table - constructor comes first" << std::endl;

		// print the constructor, which is a special method not inside "methods" vector
		output << "\tobj_" << name << " (*constructor) (";

		int i = 0;
		for (auto constructArg : constructor->argtype) {
			if (constructArg.second == name) {
				continue;
			}
			if (i == 0) {
				output << "obj_" << constructArg.second;
				++i;
			} else {
				output << ", obj_" << constructArg.second;
			}
		}
		output << ");" << std::endl;

		// print the rest of the class's methods
		for (auto method : currentClass->methods) {
			output << "\tobj_" << method->type["return"] << " (*" << method->name << ") (";
			i = 0;
			for (auto arg : method->argtype) {
				if (i == 0) {
					output << "obj_" << arg.second;
					++i;
				} else {
					output << ", obj_" << arg.second;
				}
			}
			output << ");" << std::endl;
		}
		output << "};" << indent;
	}
	output << "// -~-~-~-~- Structs End -~-~-~-~-" << indent;
}

void CodeGenerator::generateExterns(std::ostream &output) {
	output << "// -~-~-~-~- Externs Begin -~-~-~-~-" << indent;
	for (auto qclass : this->classes) {
		auto currentClass = qclass.second;
		std::string name = currentClass->name;
		Qmethod *constructor = currentClass->constructor;
		if (checkPrimitive(name)) {
			continue;
		} 

		output << "extern class_" << name << " the_class_" << name << ";" << indent;
	}
	output << "// -~-~-~-~- Externs End -~-~-~-~-" << indent;
}

void CodeGenerator::generateMethods(std::ostream &output) {
	output << "// -~-~-~-~- Methods Begin -~-~-~-~-" << indent;
	for (auto qclass : this->classes) {
		auto currentClass = qclass.second;
		std::string name = currentClass->name;
		Qmethod *constructor = currentClass->constructor;
		if (checkPrimitive(name)) {
			continue;
		}

		// time to print method definitions!
		// begin with the constructor...
		output << "// " << name << "'s constructor method definition" << std::endl;
		output << "obj_" << name << " new_" << name << "(";
		i = 0;
		for (auto constructArg : constructor->argtype) {
			if (constructArg.second == name) {
				continue;
			}
			if (i == 0) {
				output << "obj_" << constructArg.second << " " << constructArg.first;
				++i;
			} else {
				output << ", obj_" << constructArg.second << " " << constructArg.first;
			}
		}
		output << ") {" << std::endl;
		output << "\tobj_" << name << " new_thing = (obj_" << name <<
		") malloc(sizeof(struct obj_" << name << "_struct));" << std::endl;
		output << "\tnew_thing->clazz" << " = " << "the_class_" << name << ";" << std::endl;
		for (auto field : currentClass->instanceVarType) {
			if (field.first != "this") {
				output << "\tnew_thing->" << field.first << " = " << field.first << ";" << std::endl;
			} 
		}
		output << "\treturn new_thing;" << std::endl << "}" << indent;

		// ... now the rest of the methods
		output << "// " << name << "'s other method definitions" << std::endl;
		for (auto method : currentClass->methods) {
			std::string returnType = method->type["return"];
			std::string methodName = method->name;

			output << "obj_" << returnType << " " << name << "_method_" << methodName << "(";
			i = 0;
			for (auto arg : method->argtype) {
				if (arg.first == "Nothing" || arg.first == "return") {
					output << "";
					continue;
				}
				if (i == 0) {
					output << "obj_" << arg.second << " " << arg.first;
					++i;
				} else {
					output << ", obj_" << arg.second << " " << arg.first;
				}
			}
			// PRINT STATEMENTS HERE, PRINTING EMPTY FOR NOW
			output << ") {" << indent << "}" << indent;
		}
	}
	output << "// -~-~-~-~- Methods End -~-~-~-~-" << indent;
}

void CodeGenerator::generateSingletons(std::ostream &output) {
	output << "// -~-~-~-~- Singletons Begin -~-~-~-~-" << indent;
	for (auto qclass : this->classes) {
		auto currentClass = qclass.second;
		std::string name = currentClass->name;
		Qmethod *constructor = currentClass->constructor;
		if (checkPrimitive(name)) {
			continue;
		} 
		// printing out the singleton class 
		output << "// The " << name << " class (singleton version)" << std::endl;
		output << "struct class_" << name << "_struct the_class_" << name << "_struct = {" << std::endl;

		// print the singleton's constructor
		output << "\tnew_" << name << ", // constructor" << std::endl;

		// print the rest of the singleton's methods
		for (auto method : currentClass->methods) {
			output << "\t" << name << "_method_" << method->name << "," << std::endl;
		}
		output << "};" << indent;

		output << "class_" << name << " the_class_" << name << " = " << "&the_class_" << name << "_struct;";
	}
	output << indent;
	output << "// -~-~-~-~- Singletons End -~-~-~-~-" << indent;
}

bool CodeGenerator::generateMain(std::ostream &output) {
	output << "// -~-~-~-~- Main Method - it's the end! -~-~-~-~-" << std::endl;

	output << "int main(int argc, char *argv[]) {" << std::endl;

	Qclass *mainClass = this->tc->main;

	if (mainClass) {
		Qmethod *mainConstruct = mainClass->constructor;
		std::vector<AST::Node *> mainStatements = mainConstruct->stmts;
	
		if (!mainStatements.empty()) {
			for (AST::Node *stmt : mainStatements) {
				generateStatement(output, stmt);
			}
		}
	}

	output << "\n\treturn 0;\n}" << std::endl;

	return true;
}

std::string CodeGenerator::generateStatement(std::ostream &output, AST::Node *stmt) {
	Type nodeType = stmt->type;

	if (nodeType == INTCONST) {
		output << "\tobj_Int tempInt" << this->tempno <<  " = int_literal(" << stmt->value << ");" << std::endl;
		++this->tempno;
		return ("tempInt" + std::to_string(this->tempno - 1));
	}

	if (nodeType == STRCONST) {
		output << "\tobj_String tempStr" << this->tempno <<  " = str_literal(\"" << stmt->name << "\");" << std::endl;
		++this->tempno;
		return ("tempStr" + std::to_string(this->tempno - 1));
	}
}


void CodeGenerator::generateMainCall(std::ostream &output, AST::Node *stmt) {
	// this is the name of the method caller, aka the "name" in "name.METHOD()"
	AST::Node *callerIdent = stmt->get(LOAD, L_EXPR);
	// this is the case where the caller of a method is an intLiteral
	AST::Node *callerInt = stmt->get(INTCONST);
	// this is the case where the caller of a method is a strLiteral
	AST::Node *callerStr = stmt->get(STRCONST);
	AST::Node *argVecs = stmt->get(ACTUAL_ARGS);

	std::string caller;

	// handle the case where method is being called by an identifier
	if (callerIdent != NULL) {
		AST::Node *methodCallerName = callerIdent->get(IDENT, LOC);
		if (methodCallerName != NULL) {
			output << "\t" << methodCallerName->name << "->clazz->";
			caller = methodCallerName->name;
		}
	} 
	// handle the case where method is being called by an intLiteral, used in PLUS method calls.
	// going to need a temp variable for this 
	else if (callerInt != NULL) {
		output << "\t// calling a method on an int literal, need a temp variable" << std::endl;
		output << "\tobj_Int tempInt" << this->tempno <<  " = int_literal(" << callerInt->value << ");" << std::endl;
		output << "\ttempInt" << this->tempno << "->clazz->";
		caller = "tempInt" + std::to_string(this->tempno);
		++this->tempno;
	}
	// handle the case where method is being called by a strLiteral, used in PLUS method calls. (concatenation)
	// going to need a temp variable for this  
	else if (callerStr != NULL) {
		output << "\t// calling a method on a string literal, need a temp variable" << std::endl;
		output << "\tobj_String tempStr" << this->tempno <<  " = str_literal(" << callerStr->name << ");" << std::endl;
		output << "\ttempStr" << this->tempno << "->clazz->";
		caller = "tempStr" + std::to_string(this->tempno);
		++this->tempno;
	}

	// get the name of the method we're calling and output it
	AST::Node *methodName = stmt->get(IDENT, METHOD);
	output << methodName->name << "(";

	// get the arguments to the method call and begin to output them
	AST::Node *methodArguments = stmt->get(ACTUAL_ARGS);
	std::vector<AST::Node *> methodActuals = methodArguments->getAll(METHOD_ARG);
	if (!methodActuals.empty()) {
		int i = 0;
		// this will check all the methods we have just grabbed, needs to get several
		// different types of nodes to check which type of argument we have. the options
		// are identifiers (which INCLUDE booleans), intLiterals, and strLiterals. they
		// all need to be handled differently
		for (AST::Node *methodArg : methodActuals) {
			AST::Node *methodArgLoad = methodArg->get(LOAD, L_EXPR);
			AST::Node *methodArgInt = methodArg->get(INTCONST);
			AST::Node *methodArgStr = methodArg->get(STRCONST);
			// handle identifiers and booleans
			if (methodArgLoad != NULL) {
				AST::Node *actual = methodArgLoad->get(IDENT, LOC);
				if (i == 0) {
					output << caller;
					++i;
				} else {
					if (actual->name == "true") {
						output << ", lit_true";
					} else if (actual->name == "false") {
						output << ", lit_false";
					} else {
						output << ", " << actual->name;
					}
				}
			}
			// handle intLiterals
			else if (methodArgInt != NULL) {
				if (i == 0) {
					output << "int_literal(" << methodArgInt->value << ")";
					++i;
				} else {
					output << ", int_literal(" << methodArgInt->value << ")";
				}
			}
			// handle strLiterals
			else if (methodArgStr != NULL) {
				if (i == 0) {
					output << "str_literal(" << "\"" << methodArgStr->name << "\")";
					++i;
				} else {
					output << ", " << "str_literal(" << "\"" << methodArgStr->name << "\")";
				}
			}
		}
	}
	output << caller << ");" << std::endl;
}