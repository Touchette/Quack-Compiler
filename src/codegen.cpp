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

    bool methodsGenerated = this->generateMethods(fout);
    if (!methodsGenerated) {
    	report::error("code for class methods could not be generated!", CODEGENERATION);
        report::dynamicBail();
    } else if (report::ok()) {
        report::gnote("code for class methods successfully generated.", CODEGENERATION);
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

	// print out the actual struct for every class 
	generateClassdefs(output);

	return true;
}

void CodeGenerator::generateTypedefs(std::ostream &output) {
	for (auto qclass : this->classes) {
		auto currentClass = qclass.second;
		if (checkPrimitive(currentClass->name)) {
			continue;
		} 
		output << "typedef struct class_" << currentClass->name << 
		" *class_"<< currentClass->name << ";" << std::endl;

		output << "typedef struct obj_" << currentClass->name << 
		" *obj_"<< currentClass->name << ";" << std::endl;

		output << std::endl;
	}
}

void CodeGenerator::generateClassdefs(std::ostream &output) {
	for (auto qclass : this->classes) {
		Qclass *currentClass = qclass.second;
		if (checkPrimitive(currentClass->name)) {
			continue;
		}
		output << "struct class_" << currentClass->name << " {" << std::endl;
		output << "\tclass_" << currentClass->super << " super;" << std::endl;

		for (Qmethod *qmethod : currentClass->methods) {
			output << "\tobj_" << qmethod->type["return"] << " (*" <<
			qmethod->name << ") (" << "obj_" << currentClass->name;

			for (std::string argType : qmethod->args) {
				output << ", obj_" << currentClass->name;
			}
			output << ");" << std::endl;
		}
		output << "};" << indent;
		output << "struct obj_" << currentClass->name << " {" << std::endl;
		output << "\tclass_" << currentClass->name << " class;" << std::endl;
		for (auto field : currentClass->instanceVarType) {
			std::string varName = field.first;
			std::string varType = field.second;

			output << "\tobj_" << varType << 
			" field_" <<  varName << ";" <<
			std::endl;
		}
		output << "};" << indent;
	}
}

bool CodeGenerator::generateMethods(std::ostream &output) {
	for (auto qclass : this->classes) {
		Qclass *currentClass = qclass.second;
		if (checkPrimitive(currentClass->name)) {
			continue;
		}

		// comment denoting which type of methods we're printing
		output << "// " << currentClass->name << " Methods" << std::endl;

		// print the constructor method definition, returns new instance of class
		output << "obj_" << currentClass->name << " new_" << currentClass->name << "() {" <<
		std::endl;

		// print constructors, initializes the instance variables
		for (auto constructorArg : currentClass->constructor->type) {
			if (constructorArg.first != "return") {
				output << "\tobj_" << constructorArg.second << " this_" << constructorArg.first << ";" <<
				std::endl;
			}
		}
		output << "}" << indent;

		// print the method signature, including formalargs
		for (Qmethod *qmethod : currentClass->methods) {
			output << "obj_" << qmethod->type["return"] << " " << currentClass->name <<
			"_method_" << qmethod->name << "(obj_" << currentClass->name << " this";
			for (auto argType : qmethod->type) {
				if (argType.first != "return") {
					output << ", obj_" << argType.second << " arg_" << argType.first;
				}
			}
			output << ") {" << std::endl;

			// need to generate the method statements here

			output << "}" << indent;
		}
	}

	return true;
}

bool CodeGenerator::generateMain(std::ostream &output) {
	output << "// main method - it's the end!" << std::endl;

	output << "int main(int argc, char *argv[]) {" << std::endl;

	Qclass *mainClass = this->tc->main;

	Qmethod *mainConstruct = mainClass->constructor;
	std::vector<AST::Node *> mainStatements = mainConstruct->stmts;

	for (AST::Node *stmt : mainStatements) {
		Type nodeType = stmt->type;
		switch (nodeType) {
			case ASSIGN:
				generateMainAssign(output, stmt);
				break;
			case CALL:
				generateMainCall(output, stmt);
				break;
			default:
				std::cerr << "node type unsupported: " << typeString(nodeType) << std::endl;
				break;
		}
	}

	output << "\n\treturn 0;\n}" << std::endl;

	return true;
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
					// if (actual->name == "true") {
					// 	output << "lit_true";
					// 	++i;
					// } else if (actual->name == "false") {
					// 	output << "lit_false";
					// 	++i;
					// } else {
					// 	output << actual->name;
					// 	++i;
					// }
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

void CodeGenerator::generateMainAssign(std::ostream &output, AST::Node *stmt) {
	// assignment statements take the form of "name : OptionalType = expression;",
	// there are multiple cases for the right hand side of the expression. the left
	// hand side is always a name, or identifier, followed by an optional type declaration

	// the explicit declared type of the identifier in the assignment statement
	AST::Node *explicit_type = stmt->get(IDENT, TYPE_IDENT);

	// the types of nodes we can find while assigning are here
	AST::Node *left = stmt->get(IDENT, LOC);
	AST::Node *rightInt = stmt->get(INTCONST, R_EXPR);
	AST::Node *rightBoolLoad = stmt->get(LOAD, R_EXPR);
	AST::Node *rightCall = stmt->get(CALL, R_EXPR);
	AST::Node *rightConstruct = stmt->get(CONSTRUCTOR, R_EXPR);
	AST::Node *rightStr = stmt->get(STRCONST, R_EXPR);

	// begin checking all types of nodes to find out which type of assignment we have:
	// there are so many types that this ends up a massive ugly elif statement

	// check assignment of form "ident = intLiteral;"
	if ((left != NULL) && (rightInt != NULL)) {
		if (explicit_type != NULL) {
			output << "\tobj_" << explicit_type->name << " " << left->name << " = int_literal(" << rightInt->value << ");\n";
		} else {
			output << "\tobj_UNKNOWN" << " " << left->name << " = int_literal(" << rightInt->value << ");\n";
		}
	}

	// check assignment of form "ident = strLiteral;
	if ((left != NULL) && (rightStr != NULL)) {
		if (explicit_type != NULL) {
			output << "\tobj_" << explicit_type->name << " " << left->name << " = str_literal(\"" << rightStr->name << "\");\n";
		} else {
			output << "\tobj_UNKNOWN" << " " << left->name << " = str_literal(\"" << rightStr->name << "\");\n";
		}
	}
	
	// check assignment of form "ident = boolean;"
	if ((left != NULL) && (rightBoolLoad != NULL)) {
		AST::Node *rightBool = rightBoolLoad->get(IDENT, LOC);
		// true is an obj_boolean struct represented by lit_true in builtins.cpp
		if (rightBool->name == "true") {
			if (explicit_type != NULL) {
				output << "\tobj_" << explicit_type->name << " " << left->name << " = lit_true;\n";
			} else {
				output << "\tobj_UNKNOWN " << left->name << " = \"what? boolean but no identified type?\";\n";
			}
		} else if (rightBool->name == "false") {
			// true is an obj_boolean struct represented by lit_false in builtins.cpp
			if (explicit_type != NULL) {
				output << "\tobj_" << explicit_type->name << " " << left->name << " = lit_false;\n";
			} else {
				output << "\tobj_UNKNOWN " << left->name << " = \"what? boolean but no identified type?\";\n";
			}
		}
	}

	// check assignment of form "ident = methodCall();", this could be any method call including PLUS, etc
	if ((left != NULL) && (rightCall != NULL)) {
		std::string caller;

		// this is the name of the method caller, aka the "name" in "name.METHOD()"
		AST::Node *rightMethodCallerIdent = rightCall->get(LOAD, L_EXPR);
		// this is the case where the caller of a method is an intLiteral
		AST::Node *rightMethodCallerInt = rightCall->get(INTCONST);
		// this is the case where the caller of a method is a strLiteral
		AST::Node *rightMethodCallerStr = rightCall->get(STRCONST);
		if (rightMethodCallerIdent != NULL) {
			AST::Node *rightMethodCallerName = rightMethodCallerIdent->get(IDENT, LOC);
			if (rightMethodCallerName != NULL) {
				if (explicit_type != NULL) {
					output << "\tobj_" << explicit_type->name << " " << left->name << " = ";
				} else {
					output << "\tobj_UNKNOWN " << left->name << " = ";
				}
				output << rightMethodCallerName->name << "->clazz->";
				caller = rightMethodCallerName->name;
			}
		} 
		// handle the case where method is being called by an intLiteral, used in PLUS method calls.
		// going to need a temp variable for this 
		else if (rightMethodCallerInt != NULL) {
			output << "\t// adding two int consts, need a temporary variable" << std::endl;
			output << "\tobj_Int tempInt" << this->tempno << " = int_literal(" << rightMethodCallerInt->value << ");" << std::endl;
			if (explicit_type != NULL) {
				output << "\tobj_" << explicit_type->name << " " << left->name << " = tempInt" << this->tempno << "->clazz->";
			} else {
				output << "\tobj_UNKNOWN " << left->name << " = tempInt" << this->tempno << "->clazz->";
			}
			caller = "tempInt" + std::to_string(this->tempno);
			++this->tempno;
		}
		// handle the case where method is being called by a strLiteral, used in PLUS method calls. (concatenation)
		// going to need a temp variable for this  
		else if (rightMethodCallerStr != NULL) {
			output << "\t// adding two str consts, need a temporary variable" << std::endl;
			output << "\tobj_String tempStr" << this->tempno << " = str_literal(\"" << rightMethodCallerStr->name << "\");" << std::endl;\
			if (explicit_type != NULL) {
				output << "\tobj_" << explicit_type->name << " " << left->name << " = tempStr" << this->tempno << "->clazz->";
			} else {
				output << "\tobj_UNKNOWN " << left->name << " = tempStr" << this->tempno << "->clazz->";
			}
			caller = "tempStr" + std::to_string(this->tempno);
			++this->tempno;
		}

		// get the name of the method we're calling and output it
		AST::Node *rightMethodName = rightCall->get(IDENT, METHOD);
		output << rightMethodName->name << "(";

		// get the arguments to the method call and begin to output them
		AST::Node *rightArguments = rightCall->get(ACTUAL_ARGS);
		std::vector<AST::Node *> methodActuals = rightArguments->getAll(METHOD_ARG);
		if (!methodActuals.empty()) {	
			// this will check all the methods we have just grabbed, needs to get several
			// different types of nodes to check which type of argument we have. the options
			// are identifiers (which INCLUDE booleans), intLiterals, and strLiterals. they
			// all need to be handled differently
			for (AST::Node *methodArg : methodActuals) {
				AST::Node *methodArgLoad = methodArg->get(LOAD, L_EXPR);
				AST::Node *methodArgInt = methodArg->get(INTCONST);
				AST::Node *methodArgStr = methodArg->get(STRCONST);
				// handle identifiers and booleans
				output << caller;
				if (methodArgLoad != NULL) {
					AST::Node *actual = methodArgLoad->get(IDENT, LOC);
					if (actual->name == "true") {
						output << ", lit_true";
					} else if (actual->name == "false") {
						output << ", lit_false";
					} else {
						output << ", " << actual->name;
					}
				}
				// handle intLiterals
				else if (methodArgInt != NULL) {
					output << ", int_literal(" << methodArgInt->value << ")";
				}
				// handle strLiterals
				else if (methodArgStr != NULL) {
					output << "str_literal(" << "\"" << methodArgStr->name << "\")";
				}
			}
		}
		else {
			output << caller;
		}
		output << ");" << std::endl;
	}

	// practically the same as a method call, this is the call of a constructor for a class
	if ((left != NULL) && (rightConstruct != NULL)) {
		AST::Node *rightConstructor = rightConstruct->get(IDENT);
		output << "\tobj_" << rightConstructor->name << " " << left->name << " = " << "new_" << rightConstructor->name << "(";
		AST::Node *constructArgs = rightConstruct->get(ACTUAL_ARGS);
		std::vector<AST::Node *> constructorFormals = constructArgs->getAll(METHOD_ARG);
		if (!constructorFormals.empty()) {
			int i = 0;
			for (AST::Node *formalArg : constructorFormals) {
				AST::Node *formalArgLoad = formalArg->get(LOAD, L_EXPR);
				AST::Node *formalArgInt = formalArg->get(INTCONST);
				AST::Node *formalArgStr = formalArg->get(STRCONST);
				if (formalArgLoad != NULL) {
					AST::Node *formal = formalArgLoad->get(IDENT, LOC);
					if (i == 0) {
						if (formal->name == "true") {
							output << "lit_true";
							++i;
						} else if (formal->name == "false") {
							output << "lit_false";
							++i;
						} else {
							output << formal->name;
							++i;
						}
					} else {
						if (formal->name == "true") {
							output << ", lit_true";
						} else if (formal->name == "false") {
							output << ", lit_false";
						} else {
							output << ", " << formal->name;
						}
					}
				}
				else if (formalArgInt != NULL) {
					if (i == 0) {
						output << "int_literal(" <<formalArgInt->value << ")";
						++i;
					} else {
						output << ", int_literal(" << formalArgInt->value << ")";
					}
				}
				else if (formalArgStr != NULL) {
					if (i == 0) {
						output << "str_literal(\"" << formalArgStr->name << "\")";
						++i;
					} else {
						output << ", str_literal(\"" << formalArgStr->name << "\")";
					}
				}
			}
		}
		output << ");" << std::endl;
	}
}