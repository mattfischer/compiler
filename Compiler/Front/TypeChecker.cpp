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
	 * \brief Check a program's type correctness
	 * \param node Root of syntax tree
	 * \return True if types are correct
	 */
	bool TypeChecker::check(Node *node)
	{
		try {
			// Check each procedure in turn
			for(unsigned int i=0; i<node->children.size(); i++) {
				Node *child = node->children[i];
				Procedure *procedure = addProcedure(child);

				// Construct a scope for the procedure, and add the parameters into it
				Scope scope;
				for(unsigned int j=0; j<procedure->arguments.size(); j++) {
					scope.addSymbol(procedure->arguments[j]);
				}
				check(child->children[2], procedure, scope);
				child->type = TypeNone;
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
	void TypeChecker::check(Node *node, Procedure *procedure, Scope &scope)
	{
		switch(node->nodeType) {
			case Node::NodeTypeList:
			case Node::NodeTypePrint:
				checkChildren(node, procedure, scope);
				node->type = TypeNone;
				break;

			case Node::NodeTypeCall:
				{
					if(node->children[0]->nodeType == Node::NodeTypeId) {
						Procedure *procedure = findProcedure(node->children[0]->lexVal.s);
						if(procedure) {
							// Check parameter number and types
							if(procedure->arguments.size() == node->children[1]->children.size()) {
								for(unsigned int i=0; i<node->children[1]->children.size(); i++) {
									check(node->children[1]->children[i], procedure, scope);
									if(node->children[1]->children[i]->type != procedure->arguments[i]->type) {
										std::stringstream s;
										s << "Type mismatch on argument " << i << " of procedure " << procedure->name;
										throw TypeError(node, s.str());
									}
								}

								// Checks succeeded, assign return type to the call node
								node->type = procedure->returnType;
							} else {
								std::stringstream s;
								s << "Incorrect number of arguments to procedure " << procedure->name;
								throw TypeError(node, s.str());
							}
						} else {
							std::stringstream s;
							s << "Undeclared procedure " << node->children[0]->lexVal.s;
							throw TypeError(node, s.str());
						}
					} else {
						throw TypeError(node, "Function call performed on non-function");
					}
				break;
				}

			case Node::NodeTypeVarDecl:
				// Add the declared variable to the current scope
				scope.addSymbol(node->children[0]->lexVal.s, node->lexVal.s, node);
				node->type = TypeNone;
				break;

			case Node::NodeTypeAssign:
				checkChildren(node, procedure, scope);

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
				checkChildren(node, procedure, scope);
				if(node->children[0]->type != TypeBool) {
					throw TypeError(node, "Type mismatch");
				}

				node->type = TypeNone;
				break;

			case Node::NodeTypeCompare:
				checkChildren(node, procedure, scope);

				// Check that types match
				if(node->children[0]->type != TypeInt ||
					node->children[1]->type != TypeInt) {
						throw TypeError(node, "Type mismatch");
				}
				node->type = TypeBool;
				break;

			case Node::NodeTypeArith:
				checkChildren(node, procedure, scope);

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
				check(node->children[0], procedure, scope);

				// Check that return argument matches the procedure's return type
				if(node->children[0]->type != procedure->returnType) {
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
	void TypeChecker::checkChildren(Node *node, Procedure *procedure, Scope &scope)
	{
		for(unsigned int i=0; i<node->children.size(); i++) {
			check(node->children[i], procedure, scope);
		}
	}

	/*!
	 * \brief Add a symbol to the scope
	 * \param typeName Name of type
	 * \param name Name of variable
	 * \return True if add was successful, false if variable already existed
	 */
	bool TypeChecker::Scope::addSymbol(const std::string &typeName, const std::string &name, Node *node)
	{
		Type *type = Type::find(typeName);
		if(!type) {
			std::stringstream s;
			s << "Type '" << typeName << "' not found.";
			throw TypeError(node, s.str());
		}
		
		Symbol *symbol = new Symbol(type, name);
		mSymbols.push_back(symbol);

		return true;
	}

	/*!
	 * \brief Add an already-created symbol to the scope
	 * \param symbol Symbol to add
	 */
	bool TypeChecker::Scope::addSymbol(Symbol *symbol)
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
	TypeChecker::Symbol *TypeChecker::Scope::findSymbol(const std::string &name)
	{
		for(unsigned int i=0; i<mSymbols.size(); i++) {
			if(mSymbols[i]->name == name) {
				return mSymbols[i];
			}
		}

		return 0;
	}

	TypeChecker::Procedure* TypeChecker::addProcedure(Node *node)
	{
		std::string typeName = node->children[0]->lexVal.s;
		Type *returnType = Type::find(typeName);
		if(!returnType) {
			std::stringstream s;
			s << "Type '" << typeName << "' not found.";
			throw TypeError(node, s.str());
		}

		std::vector<Symbol*> arguments;
		for(unsigned int i=0; i<node->children[1]->children.size(); i++) {
			Node *decl = node->children[1]->children[i];
			std::string argTypeName = decl->children[0]->lexVal.s;
			Type *type = Type::find(argTypeName);
			if(!type) {
				std::stringstream s;
				s << "Type '" << argTypeName << "' not found.";
				throw TypeError(node, s.str());
			}
			arguments.push_back(new Symbol(type, decl->lexVal.s));
		}

		Procedure *procedure = new Procedure(returnType, node->lexVal.s, arguments);
		mProcedures.push_back(procedure);

		return procedure;
	}

	TypeChecker::Procedure *TypeChecker::findProcedure(const std::string &name)
	{
		for(unsigned int i=0; i<mProcedures.size(); i++) {
			if(mProcedures[i]->name == name) {
				return mProcedures[i];
			}
		}

		return 0;
	}
}