#include "Compiler.h"
#include "Linker.h"
#include "Assembler.h"

#include "VM/Interp.h"

#include <iostream>
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
	linked->print();
	std::cout << std::endl;

	std::cout << "*** Output ***" << std::endl;
	VM::Interp::run(linked);

	return 0;
}