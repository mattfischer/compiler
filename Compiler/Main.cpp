#include "Compiler.h"
#include "Linker.h"
#include "Assembler.h"

#include "VM/Interp.h"

#include "Util/Log.h"

#include <iostream>
#include <string>
#include <fstream>

/*!
 * \brief Compile a program, caching the assembly file
 * \param filename Input filename
 * \return Compiled program
 */
VM::Program *compileCached(const std::string &filename)
{
	std::string asmFilename = filename.substr(0, filename.find('.')) + ".asm";

	std::ifstream asmFileTest(asmFilename.c_str());
	if(asmFileTest.fail()) {
		// If assembly file was not present, compile it
		std::ofstream asmOut(asmFilename.c_str());

		Compiler compiler;
		bool success = compiler.compileToAsm(filename, asmOut);
		if(!success) {
			Util::log("error") << "Error: " << compiler.errorMessage() << std::endl;
			return 0;
		}
	}

	// Assemble the assembly file
	Assembler assembler;
	VM::Program *program = assembler.assemble(asmFilename);
	if(!program) {
		Util::log("error") << "Error: " << assembler.errorMessage() << std::endl;
		return 0;
	}

	return program;
}

int main(int arg, char *argv[])
{
	// Compile the runtime library, caching the assembly
	VM::Program *runtime = compileCached("string.lang");
	if(!runtime) {
		return 1;
	}

	// Compile the user program
	Compiler compiler;
	VM::Program *vmProgram = compiler.compile("input.lang");
	if(!vmProgram) {
		std::cerr << "Error: " << compiler.errorMessage() << std::endl;
		return 1;
	}

	// Link runtime library into program
	std::vector<VM::Program*> programs;
	programs.push_back(runtime);
	programs.push_back(vmProgram);

	Linker linker;
	VM::Program *linked = linker.link(programs);
	if(!linked) {
		std::cerr << "Error: " << linker.errorMessage() << std::endl;
		return 1;
	}

	// Print out the linked program
	Util::log("code") << "*** Code ***" << std::endl;
	linked->print(Util::log("code"));
	Util::log("code") << std::endl;

	// Run the program
	Util::log("output") << "*** Output ***" << std::endl;
	VM::Interp::run(linked, Util::log("output"));

	return 0;
}