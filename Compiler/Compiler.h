#ifndef COMPILER_H
#define COMPILER_H

#include "VM/Program.h"

#include <string>

class Compiler {
public:
	Compiler();

	VM::Program *compile(const std::string &filename);
	bool compileToAsm(const std::string &filename, std::ostream &output);

	bool error() { return mError; }
	const std::string &errorMessage() { return mErrorMessage; }

private:
	void setError(const std::string &message);

	bool mError;
	std::string mErrorMessage;
};

#endif