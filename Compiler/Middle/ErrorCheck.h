#ifndef MIDDLE_ERROR_CHECK_H
#define MIDDLE_ERROR_CHECK_H

#include "IR/Program.h"
#include "IR/Procedure.h"

#include <string>

namespace Middle {
	class ErrorCheck {
	public:
		bool check(IR::Program *program);

		IR::Procedure *errorProcedure() { return mErrorProcedure; }
		const std::string &errorMessage() { return mErrorMessage; }

	private:
		IR::Procedure *mErrorProcedure;
		std::string mErrorMessage;
	};
}
#endif