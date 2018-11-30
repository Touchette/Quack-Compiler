#include "codegen.h"
#include "cstring"

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
		// for (AST::Node *stmt : constructor->stmts) {
		// 	generateStatement(output, stmt, name);
		// }
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

			if (method->argtype.size() == 1) {
				output << "obj_" << name << " this";
			} else {
				output << "obj_" << name << " this, ";
			}
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
			output << ") {" << std::endl;
			for (auto inited : method->type) {
				if (inited.first == "return") {
					continue;
				}
				if (std::find(method->args.begin(), method->args.end(), inited.first) != method->args.end()) {
					continue;
				}
				output << "\tobj_" << inited.second << " " << inited.first << ";" << std::endl;
			}
			for (AST::Node *stmt : method->stmts) {
				generateStatement(output, stmt, method, name);
			}
			output << "}" << indent;
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

		for (auto inited : mainConstruct->type) {
				if (inited.first == "return") {
					continue;
				}
				if (std::find(mainConstruct->args.begin(), mainConstruct->args.end(), inited.first) != mainConstruct->args.end()) {
					continue;
				}
				output << "\tobj_" << inited.second << " " << inited.first << ";" << std::endl;
		}

		if (!mainStatements.empty()) {
			for (AST::Node *stmt : mainStatements) {
				generateStatement(output, stmt, mainClass->constructor);
			}
		}
	}

	output << "\n\treturn 0;\n}" << std::endl;

	return true;
}

std::string CodeGenerator::generateStatement(std::ostream &output, AST::Node *stmt, Qmethod *whichMethod, std::string whichClass) {
	Type nodeType = stmt->type;
	std::string name = whichClass;
	Qclass *currentClass;
	if (whichClass != "main") {
		Qclass *currentClass = classes[whichClass];
		name = currentClass->name;
	}

	if (nodeType == IF) {
		AST::Node *cond = stmt->get(COND)->rawChildren[0];
		if (cond != NULL) {
			std::string condName = generateStatement(output, cond, whichMethod, name);
			output << "\tif (lit_true == " << condName << ") { goto if" << this->tempno << "; }" << std::endl;
		}
		std::string ifString = "if" + std::to_string(this->tempno);
		std::string elseString = "else" + std::to_string(this->tempno);
		std::string endifString = "endif" + std::to_string(this->tempno);

		output << "\tgoto " << elseString << ";" << std::endl;
		output << "\t// if statement true part!" << std::endl;

		output << "\t" << ifString << ": ; // Null statement" << std::endl;

		AST::Node *true_stmts = stmt->get(BLOCK, TRUE_STATEMENTS);
		for (AST::Node *true_stmt : true_stmts->rawChildren) {
			std::string generatedTrue = generateStatement(output, true_stmt, whichMethod, name);
			output << "\t" << generatedTrue << std::endl;
		}
		output << "\tgoto " << endifString << ";" << std::endl;
	
		output << "\t// if statement false part!" << std::endl;

		output << "\t" << elseString << ": ; // Null statement" << std::endl;

		AST::Node *false_stmts = stmt->get(BLOCK, FALSE_STATEMENTS);
		for (AST::Node *false_stmt : false_stmts->rawChildren) {
			std::string generatedFalse = generateStatement(output, false_stmt, whichMethod, name);
			output << "\t" << generatedFalse << std::endl;
		}
		output << "\tgoto " << endifString << ";" << std::endl;

		output << "\t" << endifString << ": ; // Null statement" << std::endl;

		return "";
	}

	if (nodeType == CONSTRUCTOR) {
		AST::Node *class_name_node = stmt->get(IDENT);
		std::string retVal;
		if (class_name_node != NULL) {
			std::string class_name = class_name_node->name;
			retVal =  "new_" + class_name + "(";
			Qclass *qclass = this->classes[class_name];
			Qmethod *constructorMethod = qclass->constructor;
			int numArgsConstructor = constructorMethod->args.size();

			// make sure all our args line up
			AST::Node *actual_args_container = stmt->get(ACTUAL_ARGS);
			if (actual_args_container != NULL) {
				std::vector<AST::Node *> actual_args = actual_args_container->getAll(METHOD_ARG);
				if (!actual_args.empty()) {
					std::vector<std::string> argNames;
					std::vector<std::string> argTypes;
					for (AST::Node *arg : actual_args) {
						AST::Node *subLexpr = arg->getBySubtype(METHOD_ARG);
						std::string argName = generateStatement(output, subLexpr, whichMethod, name);
						argNames.push_back(argName);
					}

					int numArgsCallToConstructor = argNames.size();
					for (int i = 0; i < numArgsConstructor; i++) {
						std::string constructorArg = constructorMethod->args[i]; // get arg name from class we are constructing
						std::string constructorArgType = constructorMethod->argtype[constructorArg]; // get the type of that arg
						argTypes.push_back(constructorArgType);
						std::string callToConstructorType = argNames[i]; // type of arg i in our call to the constructor
					}

					for (int i = 0; i < numArgsConstructor; ++i) {
						if (i == 0) {
							retVal += ("(obj_" + argTypes[i] + ") ");
							retVal += argNames[i];
						}
						else {
							retVal += (", (obj_" + argTypes[i] + ") ");
							retVal += argNames[i];
						}
					}

					retVal += ")";
				} else { // we have a constructor call with 0 args
					if (numArgsConstructor == 0) { // if the class also has 0 args, great, just return that class name
						retVal += ")";
					}
				}
				output << "\tobj_" << class_name << " tempVar" << this->tempno << " = " << retVal << ";" << std::endl;
				std::string returned = "tempVar" + std::to_string(this->tempno);
				++this->tempno;
				return returned;
			}
		}
	}

	if (nodeType == ASSIGN) {
		AST::Node *r_expr = stmt->getBySubtype(R_EXPR);
		// assign of form "this.x = ..."
		AST::Node *left = stmt->get(DOT, L_EXPR);
		if (left != NULL) {
			std::string rhs = generateStatement(output, r_expr, whichMethod, name);
			AST::Node *load = left->get(LOAD);
			if (load != NULL) {
				if (load->get(IDENT)->name == "this") { // we have found a this.x = ... statement
					std::string instanceVar = left->get(IDENT)->name;

					output << "\tthis->" << instanceVar << " = " << rhs << ";" << std::endl;

					std::string retVal = "this->" + instanceVar;

					return retVal;
				}
			} 
		}

		// assign of form "x = ..." and "x : Clss = ..."
		left = stmt->get(IDENT, LOC);
		if (left != NULL) {
			std::string rhs = generateStatement(output, r_expr, whichMethod, name);

			std::string castType = whichMethod->type[left->name];

			output << "\t" << left->name << " = ";

			output << "(obj_" << castType << ") (" << rhs << ");" << std::endl;

			return "";
		}
	}

	if (nodeType == DOT) {
		AST::Node *load = stmt->get(LOAD);
		if (load != NULL) {
			// we have a "this.x" somewhere in a method, make appropriate checks
			std::string lhs = load->get(IDENT)->name;
			if (lhs == "this") {
				std::string instanceVar = stmt->get(IDENT)->name;
				std::string completeDot = (lhs + "->" + instanceVar);
				
				return completeDot;
			} else { // we have an "x.y" somewhere in a method
				std::string instanceVar = stmt->getBySubtype(R_EXPR)->name;
				std::string completeDot = (lhs + "->" + instanceVar);
				
				return completeDot;
			}
		} else { // if the lhs of the DOT isn't a load, we have to infer its type generically
			std::string lhsTemp = generateStatement(output, stmt->rawChildren[0], whichMethod, name);
			std::string instanceVar = stmt->getBySubtype(R_EXPR)->name;

			std::string completeDot = (lhsTemp + "." + instanceVar);

			return completeDot;
		}
	}

	if (nodeType == RETURN) {
		AST::Node *load = stmt->getBySubtype(R_EXPR);
		if (load != NULL) {
			std::string returned = generateStatement(output, load, whichMethod, name);
			for (auto tbd : whichMethod->type) {
				if (tbd.first == "return") {
					std::string returnedTypeCast = "(obj_" + tbd.second + ")";
					output << "\treturn " << returnedTypeCast << " (" << returned << ");" << std::endl;
				}
			}
		}
	}

	if (nodeType == LOAD) {
		if (stmt->get(IDENT) != NULL) { 
			std::string ident = stmt->get(IDENT)->name;
			if (ident == "this") {
				return "";
			} else if (ident == "true" || ident == "false") { 
				if (ident == "true") return "lit_true";
				else return "lit_false";
			} else {
				return "";
			}
		} else {
			// if it doesn't go straight to an ident, grab whatever it's loading (most likely a dot)
			return generateStatement(output, stmt->rawChildren[0], whichMethod, name);
		}
	}

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

	if (nodeType == IDENT) {
		if (stmt->name == "true" || stmt->name == "false") {
			output << "\tobj_Boolean tempBool" << this->tempno <<  " = " << "lit_" << stmt->name << "_struct" << std::endl;
			++this->tempno;
			return ("tempBool" + std::to_string(this->tempno - 1));
		} else {
			std::cerr << "got to ident that isn't a bool?" << std::endl;
		}
	}
}