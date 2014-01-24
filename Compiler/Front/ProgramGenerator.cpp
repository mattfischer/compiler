#include "Front/ProgramGenerator.h"

#include <sstream>

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
	 * \brief Constructor
	 * \param tree Abstract syntax tree
	 */
	ProgramGenerator::ProgramGenerator(Node *tree)
	{
		mTree = tree;
	}

	/*!
	 * \brief Generate a program
	 * \return Program structure
	 */
	Program *ProgramGenerator::generate()
	{
		try {
			// Create the root program object
			Program *program = new Program;
			program->globals = new Scope();

			// Iterate through the tree's procedure definitions
			for(unsigned int i=0; i<mTree->children.size(); i++) {
				// Construct a procedure object
				Procedure *procedure = new Procedure;
				Node *procedureNode = mTree->children[i];

				// Begin populating the procedure object
				procedure->name = procedureNode->lexVal.s;
				procedure->locals = new Scope(program->globals);

				// Iterate the tree's argument items
				Node *argumentList = procedureNode->children[1];
				std::vector<Type*> argumentTypes;
				for(unsigned int j=0; j<argumentList->children.size(); j++) {
					// Construct the argument type, and add it to the list of types
					Type *argumentType = createType(argumentList->children[j]->children[0]);
					argumentTypes.push_back(argumentType);

					// Construct a symbol for the argument, and add it to the procedure's scope and argument list
					Symbol *argument = new Symbol(argumentType, argumentList->children[j]->lexVal.s);
					procedure->locals->addSymbol(argument);
					procedure->arguments.push_back(argument);
				}

				// Construct the procedure type
				Type *returnType = createType(procedureNode->children[0]);
				TypeProcedure *procedureType = new TypeProcedure(returnType, argumentTypes);
				procedure->type = procedureType;

				// Construct the procedure symbol
				Symbol *procedureSymbol = new Symbol(procedure->type, procedure->name);
				program->globals->addSymbol(procedureSymbol);

				// Type check the body of the procedure
				procedure->body = procedureNode->children[2];
				checkType(procedure->body, procedure);
				procedureNode->type = TypeNone;

				// Add the procedure to the program's procedure list
				program->procedures.push_back(procedure);
			}
			return program;
		} catch(TypeError error) {
			// Collect the error message and line from the exception
			mErrorLine = error.node()->line;
			mErrorMessage = error.message();
			return 0;
		}
	}

	/*!
	 * \brief Search for a type named by a node
	 * \param node Node describing type
	 * \return Type
	 */
	Type *ProgramGenerator::createType(Node *node)
	{
		Type *type = 0;
		if(node->nodeType == Node::NodeTypeArray) {
			type = new TypeArray(createType(node->children[0]));
		} else {
			std::string name = node->lexVal.s;
			type = Type::find(name);
			if(!type) {
				std::stringstream s;
				s << "Type '" << name << "' not found.";
				throw TypeError(node, s.str());
			}
		}

		return type;
	}

	/*!
	 * \brief Type check a syntax tree node
	 * \param node Node to check
	 * \param procedure Procedure that contains the current node
	 */
	void ProgramGenerator::checkType(Node *node, Procedure *procedure)
	{
		switch(node->nodeType) {
			case Node::NodeTypeList:
				checkChildren(node, procedure);
				node->type = TypeNone;
				break;

			case Node::NodeTypePrint:
				checkChildren(node, procedure);
				node->type = TypeNone;
				break;

			case Node::NodeTypeCall:
				{
					// First, check all child nodes
					checkChildren(node, procedure);

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
						if(!Type::equals(argumentsNode->children[i]->type, procedureType->argumentTypes[i])) {
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
					Type *type = createType(node->children[0]);
					procedure->locals->addSymbol(new Symbol(type, node->lexVal.s));
					node->type = TypeNone;
					break;
				}

			case Node::NodeTypeAssign:
				{
					checkChildren(node, procedure);

					Node *lhs = node->children[0];
					Node *rhs = node->children[1];
					if(lhs->nodeType == Node::NodeTypeId || lhs->nodeType == Node::NodeTypeVarDecl) {
						// Search for the target variable in the current scope
						std::string name = lhs->lexVal.s;
						Symbol *symbol = procedure->locals->findSymbol(name);
						if(symbol) {
							// Confirm target type matches source type
							if(!Type::equals(symbol->type, rhs->type)) {
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
					node->type = rhs->type;
					break;
				}

			case Node::NodeTypeIf:
			case Node::NodeTypeWhile:
				checkChildren(node, procedure);
				if(!Type::equals(node->children[0]->type, TypeBool)) {
					throw TypeError(node, "Type mismatch");
				}

				node->type = TypeNone;
				break;

			case Node::NodeTypeCompare:
				checkChildren(node, procedure);

				// Check that types match
				if(!Type::equals(node->children[0]->type, TypeInt) ||
					!Type::equals(node->children[1]->type, TypeInt)) {
						throw TypeError(node, "Type mismatch");
				}
				node->type = TypeBool;
				break;

			case Node::NodeTypeArith:
				checkChildren(node, procedure);

				// Check that types match
				if(!Type::equals(node->children[0]->type, TypeInt) ||
					!Type::equals(node->children[1]->type, TypeInt)) {
						throw TypeError(node, "Type mismatch");
				}
				node->type = TypeInt;
				break;

			case Node::NodeTypeId:
				{
					// Search for the named symbol in the current scope
					std::string name = node->lexVal.s;
					Symbol *symbol = procedure->locals->findSymbol(name);
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
				checkType(node->children[0], procedure);

				// Check that return argument matches the procedure's return type
				if(!Type::equals(node->children[0]->type, procedure->type->returnType)) {
					throw TypeError(node, "Type mismatch");
				}
				node->type = TypeNone;
				break;
		}
	}

	/*!
	 * \brief Check all children of a node
	 * \param node Node to check
	 * \param procedure Procedure current node belongs to
	 */
	void ProgramGenerator::checkChildren(Node *node, Procedure *procedure)
	{
		for(unsigned int i=0; i<node->children.size(); i++) {
			checkType(node->children[i], procedure);
		}
	}
}