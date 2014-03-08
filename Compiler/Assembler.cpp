#include "Assembler.h"

#include "Back/AsmTokenizer.h"
#include "Back/AsmParser.h"

#include <fstream>
#include <sstream>

Assembler::Assembler()
{
	mError = false;
}

void Assembler::setError(const std::string &filename)
{
	mError = true;
	mErrorMessage = filename;
}

VM::Program *Assembler::assemble(std::istream &input)
{
	Back::AsmTokenizer asmTokenizer(input);
	Back::AsmParser asmParser(asmTokenizer);

	VM::Program *vmProgram = asmParser.parse();
	if(!vmProgram) {
		std::stringstream s;
		s << "line " << asmParser.errorLine() << " column " << asmParser.errorColumn() << ": " << asmParser.errorMessage() << std::endl;
		setError(s.str());
		return 0;
	}

	return vmProgram;
}

VM::Program *Assembler::assemble(const std::string &filename)
{
	std::ifstream input(filename.c_str());
	return assemble(input);
}