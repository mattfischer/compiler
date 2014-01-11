#include "Front/Tokenizer.h"
#include "Front/Parser.h"
#include "Front/TypeChecker.h"
#include "Front/IRGenerator.h"

#include "Middle/Optimizer.h"
#include "Middle/ErrorCheck.h"
#include "Back/CodeGenerator.h"

#include "VM/Instruction.h"
#include "VM/Interp.h"

#include "Analysis/InterferenceGraph.h"

#include <iostream>

int main(int arg, char *argv[])
{
	Front::Tokenizer tokenizer("input.lang");
	Front::Parser parser(tokenizer);

	Front::Node *node = parser.parse();
	if(!node) {
		std::cout << "Error, line " << parser.errorLine() << " column " << parser.errorColumn() << ": " << parser.errorMessage() << std::endl;
		return 1;
	}

	Front::TypeChecker typeChecker;
	IR::Program *irProgram = 0;
	if(typeChecker.check(node)) {
		Front::IRGenerator generator;
		irProgram = generator.generate(node);
	}

	if(!irProgram) {
		return 1;
	}

	if(!Middle::ErrorCheck::check(irProgram)) {
		return 1;
	}

	Middle::Optimizer::optimize(irProgram);

	VM::Program vmProgram = Back::CodeGenerator::generate(irProgram);
	std::cout << std::endl;
	std::cout << "Code:" << std::endl;
	for(unsigned int i = 0; i < vmProgram.instructions.size(); i++) {
		std::cout << "  " << vmProgram.instructions[i] << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Output:" << std::endl;
	VM::Interp::run(vmProgram);

	return 0;
}