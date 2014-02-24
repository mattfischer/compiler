#include "Front/HllTokenizer.h"
#include "Front/HllParser.h"
#include "Front/ProgramGenerator.h"
#include "Front/IRGenerator.h"

#include "Middle/Optimizer.h"
#include "Middle/ErrorCheck.h"

#include "Back/CodeGenerator.h"
#include "Back/AsmTokenizer.h"
#include "Back/AsmParser.h"

#include "VM/Instruction.h"
#include "VM/Interp.h"

#include "Analysis/InterferenceGraph.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

int main(int arg, char *argv[])
{
	std::ifstream stream("input.lang");
	Front::HllTokenizer tokenizer(stream);
	Front::HllParser parser(tokenizer);

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

	std::cout << "*** Parsed Program ***" << std::endl;
	program->print();
	std::cout << std::endl;

	Front::IRGenerator generator;
	IR::Program *irProgram = generator.generate(program);
	if(!irProgram) {
		return 1;
	}

	std::cout << "*** IR (before optimization) ***" << std::endl;
	irProgram->print();
	std::cout << std::endl;

	Middle::ErrorCheck errorCheck;
	if(!errorCheck.check(irProgram)) {
		std::cout << "Error, procedure " << errorCheck.errorProcedure()->name() << ": " << errorCheck.errorMessage() << std::endl;
		return 1;
	}

	Middle::Optimizer::optimize(irProgram);

	std::cout << "*** IR (after optimization) ***" << std::endl;
	irProgram->print();
	std::cout << std::endl;

	std::stringstream s;
	Back::CodeGenerator::generate(irProgram, s);
	std::cout << "*** Code ***" << std::endl;
	std::cout << s.str();

	Back::AsmTokenizer asmTokenizer(s);
	Back::AsmParser asmParser(asmTokenizer);

	VM::Program *vmProgram = asmParser.parse();
	if(!vmProgram) {
		std::cout << "Error, line " << asmParser.errorLine() << " column " << asmParser.errorColumn() << ": " << asmParser.errorMessage() << std::endl;
		return 1;
	}

	std::cout << "*** Output ***" << std::endl;
	VM::Interp::run(vmProgram);

	return 0;
}