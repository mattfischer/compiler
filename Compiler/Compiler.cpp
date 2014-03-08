#include "Compiler.h"

#include "Front/HllTokenizer.h"
#include "Front/HllParser.h"
#include "Front/ProgramGenerator.h"
#include "Front/IRGenerator.h"

#include "Middle/Optimizer.h"
#include "Middle/ErrorCheck.h"

#include "Back/CodeGenerator.h"

#include "Assembler.h"

#include <fstream>
#include <sstream>

/*!
 * \brief Constructor
 */
Compiler::Compiler()
{
	mError = false;
}

/*!
 * \brief Report an error
 * \param message Error message
 */
void Compiler::setError(const std::string &message)
{
	mError = true;
	mErrorMessage = message;
}

/*!
 * \brief Compile a file to assembly code
 * \param filename Input filename
 * \param output Stream to output to
 * \return True if success
 */
bool Compiler::compileToAsm(const std::string &filename, std::ostream &output)
{
	std::ifstream hllIn(filename.c_str());
	Front::HllTokenizer tokenizer(hllIn);
	Front::HllParser parser(tokenizer);

	Front::Node *node = parser.parse();
	if(!node) {
		std::stringstream s;
		s << "line " << parser.errorLine() << " column " << parser.errorColumn() << ": " << parser.errorMessage() << std::endl;
		setError(s.str());
		return false;
	}

	Front::ProgramGenerator programGenerator(node);
	Front::Program *program = programGenerator.generate();
	if(!program) {
		std::stringstream s;
		s << "line " << programGenerator.errorLine() << ": " << programGenerator.errorMessage() << std::endl;
		setError(s.str());
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
		std::stringstream s;
		s << "procedure " << errorCheck.errorProcedure()->name() << ": " << errorCheck.errorMessage() << std::endl;
		setError(s.str());
		return false;
	}

	Middle::Optimizer::optimize(irProgram);

	std::cout << "*** IR (after optimization) ***" << std::endl;
	irProgram->print();
	std::cout << std::endl;

	Back::CodeGenerator::generate(irProgram, output);
	return true;
}

/*!
 * \brief Compile a program
 * \param filename Input filename
 * \return Compiled program
 */
VM::Program *Compiler::compile(const std::string &filename)
{
	std::stringstream buffer;

	bool success = compileToAsm(filename, buffer);
	if(!success) {
		return 0;
	}

	std::cout << "*** Assembly ***" << std::endl;
	std::cout << buffer.str() << std::endl;

	Assembler assembler;
	VM::Program *program = assembler.assemble(buffer);
	if(assembler.error()) {
		setError(assembler.errorMessage());
		return 0;
	}

	return program;
}