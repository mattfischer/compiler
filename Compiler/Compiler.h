#ifndef COMPILER_H
#define COMPILER_H

#include "VM/Program.h"

#include <string>

/*!
 * \brief Top-level compiler class
 */
class Compiler {
public:
	Compiler();

	VM::Program *compile(const std::string &filename, const std::vector<std::string> &importFilenames);

	bool error() { return mError; }
	const std::string &errorMessage() { return mErrorMessage; }

private:
	void setError(const std::string &message);

	bool mError;
	std::string mErrorMessage;
};

#endif