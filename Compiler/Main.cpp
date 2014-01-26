#include "Front/Tokenizer.h"
#include "Front/Parser.h"
#include "Front/ProgramGenerator.h"
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

	Front::ProgramGenerator programGenerator(node);
	Front::Program *program = programGenerator.generate();
	if(!program) {
		std::cout << "Error, line " << programGenerator.errorLine() << ": " << programGenerator.errorMessage() << std::endl;
		return 1;
	}

	std::cout << "Program:" << std::endl;
	program->print();
	std::cout << std::endl;

	Front::IRGenerator generator;
	IR::Program *irProgram = generator.generate(program);
	if(!irProgram) {
		return 1;
	}

	Middle::ErrorCheck errorCheck;
	if(!errorCheck.check(irProgram)) {
		std::cout << "Error, procedure " << errorCheck.errorProcedure()->name() << ": " << errorCheck.errorMessage() << std::endl;
		return 1;
	}

	std::cout << "IR (before optimization): " << std::endl;
	irProgram->print();
	std::cout << std::endl;

	Middle::Optimizer::optimize(irProgram);

	std::cout << "IR (after optimization): " << std::endl;
	irProgram->print();
	std::cout << std::endl;

	VM::Program vmProgram = Back::CodeGenerator::generate(irProgram);
	std::cout << "Code:" << std::endl;
	for(unsigned int i = 0; i < vmProgram.instructions.size(); i++) {
		std::cout << "  " << vmProgram.instructions[i] << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Output:" << std::endl;
	VM::Interp::run(vmProgram);

	return 0;
}