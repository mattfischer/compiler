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
std::unique_ptr<VM::Program> Compiler::compile(const std::string &filename, const std::vector<std::string> &importFilenames)
{
	std::ifstream hllIn(filename.c_str());
	Front::HllTokenizer tokenizer(hllIn);
	Front::HllParser parser(tokenizer);

	Front::Node *node = parser.parse();
	if(!node) {
		std::stringstream s;
		s << "line " << parser.errorLine() << " column " << parser.errorColumn() << ": " << parser.errorMessage() << std::endl;
		setError(s.str());
		return 0;
	}

	std::vector<std::unique_ptr<Front::ExportInfo>> imports;
	std::vector<std::reference_wrapper<Front::ExportInfo>> importList;
	for(unsigned int i=0; i<importFilenames.size(); i++) {
		VM::OrcFile importFile(importFilenames[i]);
		const VM::OrcFile::Section *exportInfoSection = importFile.section("export_info");
		const VM::OrcFile::Section *exportInfoStringsSection = importFile.section("export_info.strings");
		std::unique_ptr<Front::ExportInfo> exportInfo = std::make_unique<Front::ExportInfo>(exportInfoSection->data, exportInfoStringsSection->data);
		importList.push_back(*exportInfo);
		imports.push_back(std::move(exportInfo));
	}

	Front::EnvironmentGenerator environmentGenerator(node, importList);
	if(!environmentGenerator.types() || !environmentGenerator.scope()) {
		std::stringstream s;
		s << environmentGenerator.errorLocation() << ": " << environmentGenerator.errorMessage() << std::endl;
		setError(s.str());
		return 0;
	}

	Front::ProgramGenerator programGenerator(node, environmentGenerator.types(), environmentGenerator.scope());
	std::unique_ptr<Front::Program> program = programGenerator.generate();
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
	std::unique_ptr<IR::Program> irProgram = generator.generate(*program);
	if(!irProgram) {
		return 0;
	}

	Util::log("ir") << "*** IR (before optimization) ***" << std::endl;
	irProgram->print(Util::log("ir"));
	Util::log("ir") << std::endl;

	Middle::ErrorCheck errorCheck;
	if(!errorCheck.check(*irProgram)) {
		std::stringstream s;
		s << "procedure " << errorCheck.errorProcedure()->name() << ": " << errorCheck.errorMessage() << std::endl;
		setError(s.str());
		return 0;
	}

	Middle::Optimizer::optimize(*irProgram);

	Util::log("ir") << "*** IR (after optimization) ***" << std::endl;
	irProgram->print(Util::log("ir"));
	Util::log("ir") << std::endl;

	std::stringstream buffer;
	Back::CodeGenerator::generate(*irProgram, buffer);

	Util::log("asm") << "*** Assembly ***" << std::endl;
	Util::log("asm") << buffer.str() << std::endl;

	Assembler assembler;
	std::unique_ptr<VM::Program> vmProgram = assembler.assemble(buffer);
	if(assembler.error()) {
		setError(assembler.errorMessage());
		return 0;
	}

	vmProgram->exportInfo = std::make_unique<Front::ExportInfo>(*program->types, *program->scope);

	return vmProgram;
}