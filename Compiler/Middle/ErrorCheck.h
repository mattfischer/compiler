#ifndef MIDDLE_ERROR_CHECK_H
#define MIDDLE_ERROR_CHECK_H

#include "IR/Program.h"
#include "IR/Procedure.h"

#include <string>

namespace Middle {
	/*!
	 * \brief Check for errors which require dataflow analysis
	 */
	class ErrorCheck {
	public:
		bool check(IR::Program *program);

		IR::Procedure *errorProcedure() { return mErrorProcedure; } //!< Procedure which contains error
		const std::string &errorMessage() { return mErrorMessage; } //!< Error message, if any

	private:
		IR::Procedure *mErrorProcedure; //!< Procedure which contains error
		std::string mErrorMessage; //!< Error message, if any
	};
}
#endif