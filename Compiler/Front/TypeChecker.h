#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "Front/Node.h"
#include "Front/Type.h"

#include <string>
#include <vector>

namespace Front {
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
		/*!
		 * \brief A symbol in the program
		 */
		struct Symbol {
			Type *type; //!< Type of variable
			std::string name; //!< Variable name

			/*!
			 * \brief Constructor
			 * \param _type Symbol type
			 * \param _name Symbol name
			 */
			Symbol(Type *_type, const std::string &_name) : type(_type), name(_name) {}

			/*!
			 * \brief Copy constructor
			 * \param other Copy source
			 */
			Symbol(const Symbol &other) : type(other.type), name(other.name) {}
		};

		/*!
		 * \brief A collection of variables that are in scope at some point in the program
		 */
		class Scope {
		public:
			bool addSymbol(const std::string &typeName, const std::string &name, Node *tree);
			bool addSymbol(Symbol *symbol);
			Symbol *findSymbol(const std::string &name);

		private:
			std::vector<Symbol*> mSymbols; //!< Collection of symbols
		};

		struct Procedure {
			Type *returnType;
			std::string name;
			std::vector<Symbol*> arguments;

			Procedure(Type *_returnType, const std::string &_name, const std::vector<Symbol*> _arguments) : returnType(_returnType), name(_name), arguments(_arguments) {}
		};

		std::vector<Procedure*> mProcedures;

		int mErrorLine; //!< Error line
		std::string mErrorMessage; //!< Error message

		void check(Node *tree, Procedure *procedure, Scope &scope);
		void checkChildren(Node *tree, Procedure *procedure, Scope &scope);

		Procedure *addProcedure(Node *tree);
		Procedure *findProcedure(const std::string &name);
	};
}
#endif