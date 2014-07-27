#include "Compiler.h"

#include "Front/HllTokenizer.h"
#include "Front/HllParser.h"
#include "Front/EnvironmentGenerator.h"
#include "Front/ProgramGenerator.h"
#include "Front/IRGenerator.h"
#include "Front/ExportInfo.h"

#include "Middle/Optimizer.h"
#include "Middle/ErrorCheck.h"

#include "Back/CodeGenerator.h"

#include "Util/Log.h"

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
 * \brief Compile a program
 * \param filename Input filename
 * \return Compiled program
 */
VM::Program *Compiler::compile(const std::string &filename, const std::vector<std::string> &importFilenames)
{
	std::vector<Front::ExportInfo*> imports;
	for(unsigned int i=0; i<importFilenames.size(); i++) {
		VM::OrcFile importFile(importFilenames[i]);
		const VM::OrcFile::Section *exportInfoSection = importFile.section("export_info");
		const VM::OrcFile::Section *exportInfoStringsSection = importFile.section("export_info.strings");
		Front::ExportInfo *exportInfo = new Front::ExportInfo(exportInfoSection->data, exportInfoStringsSection->data);
		imports.push_back(exportInfo);
	}

	std::ifstream hllIn(filename.c_str());
	Front::HllTokenizer tokenizer(hllIn);
	Front::HllParser parser(tokenizer, imports);

	Front::Node *node = parser.parse();
	if(!node) {
		std::stringstream s;
		s << "line " << parser.errorLine() << " column " << parser.errorColumn() << ": " << parser.errorMessage() << std::endl;
		setError(s.str());
		return 0;
	}

	Front::EnvironmentGenerator environmentGenerator(node, imports);
	if(!environmentGenerator.types() || !environmentGenerator.scope()) {
		std::stringstream s;
		s << environmentGenerator.errorLocation() << ": " << environmentGenerator.errorMessage() << std::endl;
		setError(s.str());
		return 0;
	}

	Front::ProgramGenerator programGenerator(node, environmentGenerator.types(), environmentGenerator.scope());
	Front::Program *program = programGenerator.generate();
	if(!program) {
		std::stringstream s;
		s << "line " << programGenerator.errorLine() << ": " << programGenerator.errorMessage() << std::endl;
		setError(s.str());
		return 0;
	}

	Util::log("parse") << "*** Parsed Program ***" << std::endl;
	program->print(Util::log("parse"));
	Util::log("parse") << std::endl;

	Front::IRGenerator generator;
	IR::Program *irProgram = generator.generate(program);
	if(!irProgram) {
		return 0;
	}

	Util::log("ir") << "*** IR (before optimization) ***" << std::endl;
	irProgram->print(Util::log("ir"));
	Util::log("ir") << std::endl;

	Middle::ErrorCheck errorCheck;
	if(!errorCheck.check(irProgram)) {
		std::stringstream s;
		s << "procedure " << errorCheck.errorProcedure()->name() << ": " << errorCheck.errorMessage() << std::endl;
		setError(s.str());
		return 0;
	}

	Middle::Optimizer::optimize(irProgram);

	Util::log("ir") << "*** IR (after optimization) ***" << std::endl;
	irProgram->print(Util::log("ir"));
	Util::log("ir") << std::endl;

	std::stringstream buffer;
	Back::CodeGenerator::generate(irProgram, buffer);

	Util::log("asm") << "*** Assembly ***" << std::endl;
	Util::log("asm") << buffer.str() << std::endl;

	Assembler assembler;
	VM::Program *vmProgram = assembler.assemble(buffer);
	if(assembler.error()) {
		setError(assembler.errorMessage());
		return 0;
	}

	vmProgram->exportInfo = new Front::ExportInfo(program->types, program->scope);

	return vmProgram;
}