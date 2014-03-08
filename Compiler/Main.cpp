#include "Compiler.h"
#include "Linker.h"
#include "Assembler.h"

#include "VM/Interp.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>

VM::Program *compileCached(const std::string &filename)
{
	std::string asmFilename = filename.substr(0, filename.find('.')) + ".asm";
	std::ifstream asmFileTest(asmFilename.c_str());

	if(asmFileTest.fail()) {
		std::ofstream asmOut(asmFilename.c_str());

		Compiler compiler;
		bool success = compiler.compileToAsm(filename, asmOut);
		if(!success) {
			std::cout << "Error: " << compiler.errorMessage() << std::endl;
			return 0;
		}
	}

	Assembler assembler;
	VM::Program *program = assembler.assemble(asmFilename);
	if(!program) {
		std::cout << "Error: " << assembler.errorMessage() << std::endl;
		return 0;
	}

	return program;
}

int main(int arg, char *argv[])
{
	VM::Program *runtime = compileCached("string.lang");
	if(!runtime) {
		return 1;
	}

	Compiler compiler;
	VM::Program *vmProgram = compiler.compile("input.lang");
	if(!vmProgram) {
		std::cout << "Error: " << compiler.errorMessage() << std::endl;
		return 1;
	}

	std::vector<VM::Program*> programs;
	programs.push_back(runtime);
	programs.push_back(vmProgram);

	Linker linker;
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