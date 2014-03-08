#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <iostream>
#include <string>

#include "VM/Program.h"

/*!
 * \brief Top-level assembler class
 */
class Assembler {
public:
	Assembler();

	VM::Program *assemble(std::istream &input);
	VM::Program *assemble(const std::string &filename);

	bool error() { return mError; }
	const std::string &errorMessage() { return mErrorMessage; }

private:
	void setError(const std::string &message);

	bool mError;
	std::string mErrorMessage;
};

#endif