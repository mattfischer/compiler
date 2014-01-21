#include "TypeChecker.h"

#include "Front/Node.h"
#include "Front/Type.h"

#include <exception>
#include <sstream>
#include <stdio.h>
#include <stdarg.h>

namespace Front {

	/*!
	 * \brief Exception thrown when a type error occurs
	 */
	class TypeError : public std::exception {
	public:
		/*!
		 * \brief Constructor
		 * \param node Node on which error occurred
		 * \param message Error message
		 */
		TypeError(Node *node, const std::string &message)
			: mNode(node), mMessage(message)
		{}

		Node *node() { return mNode; } //!< Node of error
		const std::string &message() { return mMessage; } //!< Error message

	private:
		Node *mNode; //!< Node
		std::string mMessage; //!< Error message
	};

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
		Scope(Scope *parent = 0, Symbol *procedure = 0);

		Symbol *procedure() { return mProcedure; } //!< Procedure containing scope
		bool addSymbol(Symbol *symbol);
		Symbol *findSymbol(const std::string &name);

	private:
		Scope *mParent; //!< Parent scope
		Symbol *mProcedure; //!< Procedure containing scope
		std::vector<Symbol*> mSymbols; //!< Collection of symbols
	};

	/*!
	 * \brief Check a program's type correctness
	 * \param node Root of syntax tree
	 * \return True if types are correct
	 */
	bool TypeChecker::check(Node *node)
	{
		try {
			Scope globalScope;

			// Check each procedure in turn
			for(unsigned int i=0; i<node->children.size(); i++) {
				Node *procedureNode = node->children[i];

				// Construct the procedure type
				Type *returnType = findType(procedureNode->children[0]);
				Node *argumentList = procedureNode->children[1];
				std::vector<Type*> argumentTypes;
				for(unsigned int i=0; i<argumentList->children.size(); i++) {
					argumentTypes.push_back(findType(argumentList->children[i]->children[0]));
				}
				TypeProcedure *procedureType = new TypeProcedure(returnType, argumentTypes);

				// Construct the procedure symbol
				Symbol *procedure = new Symbol(procedureType, procedureNode->lexVal.s);
				globalScope.addSymbol(procedure);

				// Construct a scope for the procedure, and add the parameters into it
				Scope localScope(&globalScope, procedure);
				for(unsigned int j=0; j<procedureType->argumentTypes.size(); j++) {
					localScope.addSymbol(new Symbol(procedureType->argumentTypes[j], argumentList->children[j]->lexVal.s));
				}

				// Check the procedure body
				check(procedureNode->children[2], localScope);
				procedureNode->type = TypeNone;
			}
		} catch(TypeError error) {
			// Collect the error message and line from the exception
			mErrorLine = error.node()->line;
			mErrorMessage = error.message();
			return false;
		}

		return true;
	}

	/*!
	 * \brief Check a node in the syntax tree
	 * \param node Node to check
	 * \param procedure Procedure being checked
	 * \param scope Scope of variables
	 */
	void TypeChecker::check(Node *node, Scope &scope)
	{
		switch(node->nodeType) {
			case Node::NodeTypeList:
			case Node::NodeTypePrint:
				checkChildren(node, scope);
				node->type = TypeNone;
				break;

			case Node::NodeTypeCall:
				{
					// First, check all child nodes
					checkChildren(node, scope);

					// Check that call target is a procedure type
					Node *procedureNode = node->children[0];
					if(procedureNode->type->type != Type::TypeProcedure) {
						throw TypeError(node, "Expression does not evaluate to a procedure");
					}
					TypeProcedure *procedureType = (TypeProcedure*)procedureNode->type;
					Node *argumentsNode = node->children[1];

					// Check call target has correct number of parameters
					if(procedureType->argumentTypes.size() != argumentsNode->children.size()) {
						std::stringstream s;
						s << "Expression does not evaluate to a procedure taking " << argumentsNode->children.size() << " arguments";
						throw TypeError(node, s.str());
					}

					// Check call target has correct parameter types
					for(unsigned int i=0; i<argumentsNode->children.size(); i++) {
						if(argumentsNode->children[i]->type != procedureType->argumentTypes[i]) {
							std::stringstream s;
							s << "Type mismatch on procedure argument " << i;
							throw TypeError(node, s.str());
						}
					}

					// Checks succeeded, assign return type to the call node
					node->type = procedureType->returnType;
				break;
				}

			case Node::NodeTypeVarDecl:
				{
					// Add the declared variable to the current scope
					Type *type = findType(node->children[0]);
					scope.addSymbol(new Symbol(type, node->lexVal.s));
					node->type = TypeNone;
					break;
				}

			case Node::NodeTypeAssign:
				checkChildren(node, scope);

				if(node->children[0]->nodeType == Node::NodeTypeId) {
					// Search for the target variable in the current scope
					std::string name = node->children[0]->lexVal.s;
					Symbol *symbol = scope.findSymbol(name);
					if(symbol) {
						// Confirm target type matches source type
						if(symbol->type != node->children[1]->type) {
							throw TypeError(node, "Type mismatch");
						}
					} else {
						std::stringstream s;
						s << "Undeclared variable '" << name << "'";
						throw TypeError(node, s.str());
					}
				} else {
					throw TypeError(node, "Lvalue required");
				}
				node->type = node->children[1]->type;
				break;

			case Node::NodeTypeIf:
			case Node::NodeTypeWhile:
				checkChildren(node, scope);
				if(node->children[0]->type != TypeBool) {
					throw TypeError(node, "Type mismatch");
				}

				node->type = TypeNone;
				break;

			case Node::NodeTypeCompare:
				checkChildren(node, scope);

				// Check that types match
				if(node->children[0]->type != TypeInt ||
					node->children[1]->type != TypeInt) {
						throw TypeError(node, "Type mismatch");
				}
				node->type = TypeBool;
				break;

			case Node::NodeTypeArith:
				checkChildren(node, scope);

				// Check that types match
				if(node->children[0]->type != TypeInt ||
					node->children[1]->type != TypeInt) {
						throw TypeError(node, "Type mismatch");
				}
				node->type = TypeInt;
				break;

			case Node::NodeTypeId:
				{
					// Search for the named symbol in the current scope
					std::string name = node->lexVal.s;
					Symbol *symbol = scope.findSymbol(name);
					if(!symbol) {
						std::stringstream s;
						s << "Undefined variable '" << name << "'";
						throw TypeError(node, s.str());
					}

					node->type = symbol->type;
					break;
				}

			case Node::NodeTypeConstant:
				node->type = TypeInt;
				break;

			case Node::NodeTypeReturn:
				check(node->children[0], scope);

				// Check that return argument matches the procedure's return type
				TypeProcedure *procedureType = (TypeProcedure*)scope.procedure()->type;
				if(node->children[0]->type != procedureType->returnType) {
					throw TypeError(node, "Type mismatch");
				}
				node->type = TypeNone;
				break;
		}
	}

	/*!
	 * \brief Check the children of a node
	 * \param node Node to check
	 * \param procedure Current procedure
	 * \param scope Active scope
	 */
	void TypeChecker::checkChildren(Node *node, Scope &scope)
	{
		for(unsigned int i=0; i<node->children.size(); i++) {
			check(node->children[i], scope);
		}
	}

	/*!
	 * \brief Search for a type named by a node
	 * \param node Node describing type
	 * \return Type
	 */
	Type *TypeChecker::findType(Node *node)
	{
		std::string name = node->lexVal.s;
		Type *type = Type::find(name);
		if(!type) {
			std::stringstream s;
			s << "Type '" << name << "' not found.";
			throw TypeError(node, s.str());
		}

		return type;
	}

	/*!
	* \brief Constructor
	* \param parent Parent scope
	* \param procedure Procedure containing scope
	*/
	Scope::Scope(Scope *parent, Symbol *procedure)
	{
		mParent = parent;
		mProcedure = procedure;
	}

	/*!
	 * \brief Add an already-created symbol to the scope
	 * \param symbol Symbol to add
	 */
	bool Scope::addSymbol(Symbol *symbol)
	{
		Symbol *localSymbol = new Symbol(*symbol);
		mSymbols.push_back(localSymbol);

		return true;
	}

	/*!
	 * \brief Search for a symbol in the scope
	 * \param name Name of symbol
	 * \return Symbol if found, or 0 if not
	 */
	Symbol *Scope::findSymbol(const std::string &name)
	{
		for(unsigned int i=0; i<mSymbols.size(); i++) {
			if(mSymbols[i]->name == name) {
				return mSymbols[i];
			}
		}

		if(mParent) {
			// Search parent scope
			return mParent->findSymbol(name);
		} else {
			return 0;
		}
	}
}