#include "Assembler.h"

#include "Back/AsmTokenizer.h"
#include "Back/AsmParser.h"

#include <fstream>
#include <sstream>

/*!
 * \brief Constructor
 */
Assembler::Assembler()
{
	mError = false;
}

/*!
 * \brief Report an error
 * \param message Error message
 */
void Assembler::setError(const std::string &message)
{
	mError = true;
	mErrorMessage = message;
}

/*!
 * \brief Assemble a file
 * \param input Input stream
 * \return Assembled program, or 0 if error
 */
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

/*!
 * \brief Assemble a file
 * \param input Input file
 * \return Assembled program, or 0 if error
 */
VM::Program *Assembler::assemble(const std::string &filename)
{
	std::ifstream input(filename.c_str());
	return assemble(input);
}