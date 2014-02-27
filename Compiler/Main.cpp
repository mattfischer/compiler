#include "Front/HllTokenizer.h"
#include "Front/HllParser.h"
#include "Front/ProgramGenerator.h"
#include "Front/IRGenerator.h"

#include "Middle/Optimizer.h"
#include "Middle/ErrorCheck.h"

#include "Back/CodeGenerator.h"
#include "Back/AsmTokenizer.h"
#include "Back/AsmParser.h"
#include "Back/Linker.h"

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

	std::vector<VM::Program*> programs;
	programs.push_back(vmProgram);

	Back::Linker linker;
	VM::Program *linked = linker.link(programs);
	if(!linked) {
		std::cout << "Error: " << linker.errorMessage() << std::endl;
		return 1;
	}

	for(unsigned int i = 0; i < linked->instructions.size(); i+=4) {
		VM::Instruction instr;
		for(std::map<std::string, int>::iterator it=linked->symbols.begin(); it != linked->symbols.end(); it++) {
			if(i == it->second) {
				std::cout << it->first << ":" << std::endl;
				break;
			}
		}

		std::cout << "  ";
		for(int j=0; j<4; j++) {
			int d = 0;
			if(i + j < linked->instructions.size()) {
				d = linked->instructions[i + j];
			}
			std::cout << std::setw(2) << std::setfill('0') << std::setbase(16) << d;
		}
		std::cout << std::setbase(10);
		std::memcpy(&instr, &linked->instructions[i], 4);
		std::cout << "  " << instr << std::endl;
	}
	std::cout << std::endl;

	std::cout << "*** Output ***" << std::endl;
	VM::Interp::run(linked);

	return 0;
}