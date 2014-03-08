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
#include <string>

bool compile(std::istream &hllIn, std::ostream &asmOut)
{
	Front::HllTokenizer tokenizer(hllIn);
	Front::HllParser parser(tokenizer);

	Front::Node *node = parser.parse();
	if(!node) {
		std::cout << "Error, line " << parser.errorLine() << " column " << parser.errorColumn() << ": " << parser.errorMessage() << std::endl;
		return false;
	}

	Front::ProgramGenerator programGenerator(node);
	Front::Program *program = programGenerator.generate();
	if(!program) {
		std::cout << "Error, line " << programGenerator.errorLine() << ": " << programGenerator.errorMessage() << std::endl;
		return false;
	}

	std::cout << "*** Parsed Program ***" << std::endl;
	program->print();
	std::cout << std::endl;

	Front::IRGenerator generator;
	IR::Program *irProgram = generator.generate(program);
	if(!irProgram) {
		return false;
	}

	std::cout << "*** IR (before optimization) ***" << std::endl;
	irProgram->print();
	std::cout << std::endl;

	Middle::ErrorCheck errorCheck;
	if(!errorCheck.check(irProgram)) {
		std::cout << "Error, procedure " << errorCheck.errorProcedure()->name() << ": " << errorCheck.errorMessage() << std::endl;
		return false;
	}

	Middle::Optimizer::optimize(irProgram);

	std::cout << "*** IR (after optimization) ***" << std::endl;
	irProgram->print();
	std::cout << std::endl;

	Back::CodeGenerator::generate(irProgram, asmOut);

	return true;
}

VM::Program *assemble(std::istream &asmIn)
{
	Back::AsmTokenizer asmTokenizer(asmIn);
	Back::AsmParser asmParser(asmTokenizer);

	VM::Program *vmProgram = asmParser.parse();
	if(!vmProgram) {
		std::cout << "Error, line " << asmParser.errorLine() << " column " << asmParser.errorColumn() << ": " << asmParser.errorMessage() << std::endl;
		return 0;
	}

	return vmProgram;
}

VM::Program *compile(const std::string &filename)
{
	std::ifstream hllIn(filename.c_str());
	std::stringstream buffer;
	compile(hllIn, buffer);

	std::cout << "*** Assembly ***" << std::endl;
	std::cout << buffer.str() << std::endl;

	return assemble(buffer);
}

VM::Program *compileCached(const std::string &filename)
{
	std::ifstream hllIn(filename.c_str());

	std::string asmFilename = filename.substr(0, filename.find('.')) + ".asm";
	std::ifstream asmFileTest(asmFilename.c_str());

	if(asmFileTest.fail()) {
		std::ofstream asmOut(asmFilename.c_str());
		compile(hllIn, asmOut);
	}

	std::ifstream asmIn(asmFilename.c_str());
	return assemble(asmIn);
}

int main(int arg, char *argv[])
{
	VM::Program *runtime = compileCached("string.lang");
	if(!runtime) {
		return 1;
	}
	VM::Program *vmProgram = compile("input.lang");
	if(!vmProgram) {
		return 1;
	}

	std::vector<VM::Program*> programs;
	programs.push_back(runtime);
	programs.push_back(vmProgram);

	Back::Linker linker;
	VM::Program *linked = linker.link(programs);
	if(!linked) {
		std::cout << "Error: " << linker.errorMessage() << std::endl;
		return 1;
	}

	std::cout << "*** Code ***" << std::endl;
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