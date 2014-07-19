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
VM::Program *compileCached(const std::string &outputFilename, const std::vector<std::string> &sourceFilenames)
{
	std::ifstream outputFileTest(outputFilename.c_str());
	if(outputFileTest.fail()) {
		// If runtime file was not present, compile it
		Compiler compiler;
		std::vector<VM::Program*> programs;
		for(unsigned int i=0; i<sourceFilenames.size(); i++) {
			VM::Program *program = compiler.compile(sourceFilenames[i]);
			if(!program) {
				Util::log("error") << "Error: " << compiler.errorMessage() << std::endl;
				return 0;
			}
			programs.push_back(program);
		}

		Linker linker;
		VM::Program *linked = linker.link(programs);
		std::ofstream outputFile(outputFilename.c_str());
		linked->write(outputFile);
		outputFile.close();
	}

	// Load the runtime binary
	std::ifstream fileIn(outputFilename.c_str());
	VM::Program *program = new VM::Program(fileIn);
	return program;
}

int main(int arg, char *argv[])
{
	// Compile the runtime library and cache to a binary
	std::vector<std::string> sourceFilenames;
	sourceFilenames.push_back("string.lang");
	VM::Program *runtime = compileCached("runtime.orc", sourceFilenames);
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