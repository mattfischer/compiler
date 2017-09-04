#include "Compiler.h"
#include "Linker.h"
#include "Assembler.h"

#include "VM/Interp.h"

#include "Util/Log.h"

#include <iostream>
#include <string>
#include <fstream>

/*!
 * \brief Compile the runtime if not already present
 * \param runtimeFilename Filename to store runtime in
 * \return True if success
 */
bool compileRuntime(const std::string &runtimeFilename)
{
	std::ifstream runtimeFileTest(runtimeFilename.c_str());
	if(runtimeFileTest.fail()) {
		// If runtime file was not present, compile it
		std::vector<std::string> sourceFilenames;
		std::vector<std::string> importFilenames;
		sourceFilenames.push_back("string.lang");
		sourceFilenames.push_back("System.lang");

		Compiler compiler;
		std::vector<std::unique_ptr<VM::Program>> programs;
		std::vector<const VM::Program*> programList;
		for(unsigned int i=0; i<sourceFilenames.size(); i++) {
			std::unique_ptr<VM::Program> program = compiler.compile(sourceFilenames[i], importFilenames);
			if(!program) {
				std::cerr << "Error: " << compiler.errorMessage() << std::endl;
				return false;
			}
			programList.push_back(program.get());
			programs.push_back(std::move(program));
		}

		Linker linker;
		std::unique_ptr<VM::Program> linked = linker.link(programList);
		if(!linked) {
			std::cerr << "Error: " << linker.errorMessage() << std::endl;
			return false;
		}
		linked->write(runtimeFilename);
	}

	return true;
}

int main(int arg, char *argv[])
{
	// Ensure that the runtime is compiled
	std::string runtimeFilename = "runtime.orc";
	if(!compileRuntime(runtimeFilename)) {
		return 1;
	}

	// Compile the user program
	Compiler compiler;
	std::vector<std::string> importFilenames;
	importFilenames.push_back(runtimeFilename);
	std::unique_ptr<VM::Program> vmProgram = compiler.compile("input.lang", importFilenames);
	if(!vmProgram) {
		std::cerr << "Error: " << compiler.errorMessage() << std::endl;
		return 1;
	}

	// Load the runtime binary
	VM::Program *runtime = new VM::Program(runtimeFilename);

	// Link runtime library into program
	std::vector<const VM::Program*> programs;
	programs.push_back(runtime);
	programs.push_back(vmProgram.get());

	Linker linker;
	std::unique_ptr<VM::Program> linked = linker.link(programs);
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
	VM::Interp::run(*linked, Util::log("output"));

	return 0;
}