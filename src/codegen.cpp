#include "codegen.h"

bool CodeGenerator::generate() {
	primitives.push_back("String");
	primitives.push_back("Boolean");
	primitives.push_back("Int");
	primitives.push_back("Obj");
	primitives.push_back("Nothing");

	std::ofstream fout(filename);

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
	// include the built-in functions and classes provided by Professor Young
	output << "#include \"Builtins.h\"" << indent;

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
			output << "\tobj_" << qmethod->type["return"] << "\t(*" <<
			qmethod->name << ") (" << "obj_" << currentClass->name;

			for (std::string argType : qmethod->args) {
				output << ", obj_" << currentClass->name;
			}
			output << ");" << std::endl;
		}
		output << "};" << indent;
		output << "struct obj_" << currentClass->name << " {" << std::endl;
		output << "\tclass_" << currentClass->name << "\tclass;" << std::endl;
		for (auto field : currentClass->instanceVarType) {
			std::string varName = field.first;
			std::string varType = field.second;

			output << "\tobj_" << varType << 
			"\tfield_" <<  varName << ";" <<
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
		output << "// " << currentClass->name << " Methods" << std::endl;

		output << "obj_" << currentClass->name << " new_" << currentClass->name << "() {" <<
		indent << "}" << indent;
		for (Qmethod *qmethod : currentClass->methods) {
			output << "obj_" << qmethod->type["return"] << " " << currentClass->name <<
			"_method_" << qmethod->name << "(obj_" << currentClass->name << " this";
			for (std::string argType : qmethod->args) {
				output << ", obj_" << currentClass->name << " arg_" << argType;
			}
			output << ") {" << indent << "}" << indent;
		}
	}

	return true;
}

bool CodeGenerator::generateMain(std::ostream &output) {

	output << std::endl;

	output << "int main(int argc, char *argv[]) {" << std::endl;
	output << "\treturn 0;\n}" << std::endl;

	return true;
}