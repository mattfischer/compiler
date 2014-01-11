#ifndef MIDDLE_ERROR_CHECK_H
#define MIDDLE_ERROR_CHECK_H

#include <string>

namespace IR {
	class Program;
	class Procedure;
}

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