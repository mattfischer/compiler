#ifndef FRONT_PROGRAM_GENERATOR_H
#define FRONT_PROGRAM_GENERATOR_H

#include "Front/Node.h"
#include "Front/Program.h"

namespace Front {
	/*!
	 * \brief Generate a program from an abstract syntax tree
	 */
	class ProgramGenerator
	{
	public:
		ProgramGenerator(Node *tree);

		Program *generate();

		std::string errorMessage() { return mErrorMessage; } //!< Error message
		int errorLine() { return mErrorLine; } //!< Error line

	private:
		Node *mTree; //!< Abstract syntax tree

		std::string mErrorMessage; //!< Error message
		int mErrorLine; //!< Error line

		void checkType(Node *node, Procedure *procedure);
		void checkChildren(Node *node, Procedure *procedure);
		Type *createType(Node *node);
	};
}
#endif