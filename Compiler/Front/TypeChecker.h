#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "Front/Node.h"
#include "Front/Type.h"
#include "Front/Scope.h"

#include <string>
#include <vector>

namespace Front {
	class Scope;

	/*!
	 * \brief Checks the type correctness of the syntax tree, and assigns types to each node
	 */
	class TypeChecker
	{
	public:
		bool check(Node *tree);

		int errorLine() { return mErrorLine; } //!< Line of error
		const std::string &errorMessage() { return mErrorMessage; } //!< Error message, if any

	private:
		int mErrorLine; //!< Error line
		std::string mErrorMessage; //!< Error message

		void check(Node *tree, Scope &scope);
		void checkChildren(Node *tree, Scope &scope);

		Type *findType(Node *node);
	};
}
#endif