#ifndef BACK_LINKER_H
#define BACK_LINKER_H

#include "VM/Program.h"

#include <vector>

namespace Back {

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

}
#endif