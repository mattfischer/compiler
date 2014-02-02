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
					if(Type::equals(argumentType, TypeVoid)) {
						throw TypeError(argumentList->children[j], "Cannot declare procedure argument of type void");
					}
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

				// Create a context for the procedure
				Context context;
				context.procedure = procedure;
				context.scope = procedure->locals;
				context.inLoop = false;

				// Type check the body of the procedure
				procedure->body = procedureNode->children[2];
				checkType(procedure->body, context);
				procedureNode->type = TypeVoid;

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
			type = createType(node->children[0]);
			if(Type::equals(type, TypeVoid)) {
				throw TypeError(node, "Cannot declare array of voids");
			}
			type = new TypeArray(type);
		} else {
			std::string name = node->lexVal.s;
			type = Type::find(name);
			if(!type) {
				std::stringstream s;
				s << "Type '" << name << "' not found.";
				throw TypeError(node, s.str());
			}
		}

		node->type = type;
		return type;
	}

	/*!
	 * \brief Type check a syntax tree node
	 * \param node Node to check
	 * \param procedure Procedure that contains the current node
	 */
	void ProgramGenerator::checkType(Node *node, Context &context)
	{
		switch(node->nodeType) {
			case Node::NodeTypeList:
				checkChildren(node, context);
				node->type = TypeVoid;
				break;

			case Node::NodeTypePrint:
				checkChildren(node, context);
				node->type = TypeVoid;
				break;

			case Node::NodeTypeCall:
				{
					// First, check all child nodes
					checkChildren(node, context);

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
					if(Type::equals(type, TypeVoid)) {
						throw TypeError(node, "Cannot declare variable of void type");
					}

					Symbol *symbol = new Symbol(type, node->lexVal.s);
					context.scope->addSymbol(symbol);
					node->type = type;
					node->symbol = symbol;
					break;
				}

			case Node::NodeTypeAssign:
				{
					checkChildren(node, context);

					Node *lhs = node->children[0];
					Node *rhs = node->children[1];
					switch(lhs->nodeType) {
						case Node::NodeTypeId:
						case Node::NodeTypeVarDecl:
						case Node::NodeTypeArray:
							// Confirm target type matches source type
							if(!Type::equals(lhs->type, rhs->type)) {
								throw TypeError(node, "Type mismatch");
							}
							break;
						default:
							throw TypeError(node, "Lvalue required");
					}
					node->type = rhs->type;
					break;
				}

			case Node::NodeTypeIf:
				{
					Context childContext = context;
					childContext.scope = new Scope(context.scope);

					checkChildren(node, childContext);
					if(!Type::equals(node->children[0]->type, TypeBool)) {
						throw TypeError(node, "Type mismatch");
					}

					node->type = TypeVoid;
					break;
				}

			case Node::NodeTypeWhile:
				{
					Context childContext = context;
					childContext.scope = new Scope(context.scope);
					childContext.inLoop = true;

					checkChildren(node, childContext);
					if(!Type::equals(node->children[0]->type, TypeBool)) {
						throw TypeError(node, "Type mismatch");
					}

					node->type = TypeVoid;
					break;
				}

			case Node::NodeTypeFor:
				{
					Context childContext = context;
					childContext.scope = new Scope(context.scope);
					childContext.inLoop = true;

					checkChildren(node, childContext);
					if(!Type::equals(node->children[1]->type, TypeBool)) {
						throw TypeError(node, "Type mismatch");
					}

					node->type = TypeVoid;
					break;
				}

			case Node::NodeTypeCompare:
				checkChildren(node, context);

				// Check that types match
				if(!Type::equals(node->children[0]->type, node->children[1]->type)) {
					throw TypeError(node, "Type mismatch");
				}

				if(!Type::equals(node->children[0]->type, TypeInt) &&
					!Type::equals(node->children[0]->type, TypeBool)) {
					throw TypeError(node, "Type mismatch");
				}

				switch(node->nodeSubtype) {
					case Node::NodeSubtypeAnd:
					case Node::NodeSubtypeOr:
						if(!Type::equals(node->children[0]->type, TypeBool)) {
							throw TypeError(node, "Type mismatch");
						}
						break;
				}

				node->type = TypeBool;
				break;

			case Node::NodeTypeArith:
				checkChildren(node, context);

				// Check that types match
				for(unsigned int i=0; i<node->children.size(); i++) {
					if(!Type::equals(node->children[i]->type, TypeInt)) {
						throw TypeError(node->children[i], "Type mismatch");
					}
				}
				node->type = TypeInt;
				break;

			case Node::NodeTypeId:
				{
					// Search for the named symbol in the current scope
					std::string name = node->lexVal.s;
					Symbol *symbol = context.scope->findSymbol(name);
					if(!symbol) {
						std::stringstream s;
						s << "Undefined variable '" << name << "'";
						throw TypeError(node, s.str());
					}

					node->type = symbol->type;
					node->symbol = symbol;
					break;
				}

			case Node::NodeTypeConstant:
				// Type provided by parser
				break;

			case Node::NodeTypeReturn:
				if(Type::equals(context.procedure->type->returnType, TypeVoid)) {
					throw TypeError(node, "Return statement in void procedure");
				}

				checkType(node->children[0], context);

				// Check that return argument matches the procedure's return type
				if(!Type::equals(node->children[0]->type, context.procedure->type->returnType)) {
					throw TypeError(node, "Type mismatch");
				}
				node->type = TypeVoid;
				break;

			case Node::NodeTypeNew:
				{
					Node *typeNode = node->children[0];
					typeNode->type = createType(typeNode);
					node->type = typeNode->type;

					if(typeNode->nodeType == Node::NodeTypeArray && typeNode->children.size() > 1) {
						checkType(typeNode->children[1], context);
						if(!Type::equals(typeNode->children[1]->type, TypeInt)) {
							throw TypeError(typeNode->children[1], "Non-integral type used for array size");
						}
					}
					break;
				}

			case Node::NodeTypeArray:
				{
					checkChildren(node, context);
					Node *arrayNode = node->children[0];
					Node *subscriptNode = node->children[1];

					if(arrayNode->type->type != Type::TypeArray) {
						throw TypeError(arrayNode, "Attempt to take subscript of non-array");
					}

					if(!Type::equals(subscriptNode->type, TypeInt)) {
						throw TypeError(subscriptNode, "Non-integral subscript");
					}

					TypeArray *typeArray = (TypeArray*)arrayNode->type;
					node->type = typeArray->baseType;
					break;
				}

			case Node::NodeTypeBreak:
				if(!context.inLoop) {
					throw TypeError(node, "Break statement outside of loop");
				}
				node->type = TypeVoid;
				break;

			case Node::NodeTypeContinue:
				if(!context.inLoop) {
					throw TypeError(node, "Continue statement outside of loop");
				}
				node->type = TypeVoid;
				break;
		}
	}

	/*!
	 * \brief Check all children of a node
	 * \param node Node to check
	 * \param procedure Procedure current node belongs to
	 */
	void ProgramGenerator::checkChildren(Node *node, Context &context)
	{
		for(unsigned int i=0; i<node->children.size(); i++) {
			checkType(node->children[i], context);
		}
	}
}