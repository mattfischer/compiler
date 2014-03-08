#ifndef LINKER_H
#define LINKER_H

#include "VM/Program.h"

#include <vector>

/*!
 * \brief Top-level linker class
 */
class Linker {
public:
	Linker();

	VM::Program *link(const std::vector<VM::Program*> &programs);

	bool error() { return mError; }
	std::string errorMessage() { return mErrorMessage; }

private:
	bool mError;
	std::string mErrorMessage;
};

#endif