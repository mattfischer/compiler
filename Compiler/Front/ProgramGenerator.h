#ifndef FRONT_PROGRAM_GENERATOR_H
#define FRONT_PROGRAM_GENERATOR_H

#include "Front/Node.h"
#include "Front/Program.h"

#include <memory>

namespace Front {
	/*!
	 * \brief Generate a program from an abstract syntax tree
	 */
	class ProgramGenerator
	{
	public:
		ProgramGenerator(Node *tree, Types *types, Scope *scope);

		std::unique_ptr<Program> generate();

		std::string errorMessage() { return mErrorMessage; } //!< Error message
		int errorLine() { return mErrorLine; } //!< Error line

	private:
		Node *mTree; //!< Abstract syntax tree
		Types *mTypes;
		Scope *mScope;

		std::string mErrorMessage; //!< Error message
		int mErrorLine; //!< Error line

		struct Context {
			Procedure &procedure;
			Types *types;
			Scope *scope;
			bool inLoop;
		};

		void checkType(Node *node, Context &context);
		void checkChildren(Node *node, Context &context);
		Type *createType(Node *node, Types *types);
		void addProcedure(Node *node, Program &program, Scope *scope, bool instanceMethod);
	};
}
#endif