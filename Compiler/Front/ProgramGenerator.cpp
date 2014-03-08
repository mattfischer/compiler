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
			program->types = new Types();

			// Iterate through the tree's procedure definitions
			for(unsigned int i=0; i<mTree->children.size(); i++) {
				Node *node = mTree->children[i];

				if(node->nodeType == Node::NodeTypeProcedureDef) {
					generateProcedure(mTree->children[i], program, mTree->children[i]->lexVal.s);
				} else if(node->nodeType == Node::NodeTypeStructDef) {
					addStruct(mTree->children[i], program);
				} else if(node->nodeType == Node::NodeTypeClassDef) {
					addClass(mTree->children[i], program);
				}
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
	 * \brief Coerce a node to a specified type
	 * \param node Node to coerce
	 * \param type Desired type
	 * \return New node
	 */
	Node *coerce(Node *node, Type *type)
	{
		// If node is already of the given type, do nothing
		if(!Type::equals(node->type, type)) {
			bool valid = false;

			if(Type::equals(type, Types::intrinsic(Types::String))) {
				if(Type::equals(node->type, Types::intrinsic(Types::Bool)) ||
				   Type::equals(node->type, Types::intrinsic(Types::Int)) ||
				   Type::equals(node->type, Types::intrinsic(Types::Char))) {
					   valid = true;
				}
			}

			if(!valid) {
				std::stringstream s;
				s << "Cannot convert type " << node->type->name << " to " << type->name;
				throw TypeError(node, s.str());
			}

			Node *coerce = new Node();
			coerce->nodeType = Node::NodeTypeCoerce;
			coerce->nodeSubtype = Node::NodeSubtypeNone;
			coerce->line = node->line;
			coerce->type = type;
			coerce->children.push_back(node);
			node = coerce;
		}

		return node;
	}

	/*!
	 * \brief Coerce all children of a node to a given type
	 * \param node Node to process
	 * \param type Type to coerce to
	 */
	void coerceChildren(Node *node, Type *type)
	{
		for(unsigned int i=0; i<node->children.size(); i++) {
			node->children[i] = coerce(node->children[i], type);
		}
	}

	/*!
	 * \brief Determine if any child of node is of the given type
	 * \param node Node to examine
	 * \param type Type to test for
	 * \return True if there is a child of the given type
	 */
	bool isChildOfType(Node *node, Type *type)
	{
		for(unsigned int i=0; i<node->children.size(); i++) {
			if(Type::equals(node->children[i]->type, type)) {
				return true;
			}
		}

		return false;
	}

	/*!
	 * \brief Generate a single procedure
	 * \param node Tree node for procedure definition
	 * \param program Program to add procedure to
	 */
	Procedure *ProgramGenerator::generateProcedure(Node *node, Program *program, const std::string &name)
	{
		// Construct a procedure object
		Procedure *procedure = new Procedure;

		// Begin populating the procedure object
		procedure->name = name;
		procedure->locals = new Scope(program->globals);

		// Iterate the tree's argument items
		Node *argumentList = node->children[1];
		std::vector<Type*> argumentTypes;
		for(unsigned int j=0; j<argumentList->children.size(); j++) {
			// Construct the argument type, and add it to the list of types
			Type *argumentType = createType(argumentList->children[j]->children[0], program->types);
			if(Type::equals(argumentType, Types::intrinsic(Types::Void))) {
				throw TypeError(argumentList->children[j], "Cannot declare procedure argument of type void");
			}
			argumentTypes.push_back(argumentType);

			// Construct a symbol for the argument, and add it to the procedure's scope and argument list
			Symbol *argument = new Symbol(argumentType, argumentList->children[j]->lexVal.s);
			procedure->locals->addSymbol(argument);
			procedure->arguments.push_back(argument);
		}

		// Construct the procedure type
		Type *returnType = createType(node->children[0], program->types);
		procedure->type = new TypeProcedure(returnType, argumentTypes);

		// Construct the procedure symbol
		Symbol *symbol = new Symbol(procedure->type, procedure->name);
		program->globals->addSymbol(symbol);

		// Create a context for the procedure
		Context context;
		context.procedure = procedure;
		context.types = program->types;
		context.scope = procedure->locals;
		context.inLoop = false;

		// Type check the body of the procedure
		procedure->body = node->children[2];
		checkType(procedure->body, context);
		node->type = Types::intrinsic(Types::Void);

		// Add the procedure to the program's procedure list
		program->procedures.push_back(procedure);

		return procedure;
	}

	/*!
	 * \brief Add a structure to the type list
	 * \param node Node describing structure
	 * \param program Program to add to
	 */
	void ProgramGenerator::addStruct(Node *node, Program *program)
	{
		TypeStruct *type = new TypeStruct(Type::TypeStruct, node->lexVal.s);
		Node *members = node->children[0];
		for(unsigned int i=0; i<members->children.size(); i++) {
			Node *memberNode = members->children[i];
			Type *memberType = createType(memberNode->children[0], program->types);
			type->addMember(memberType, memberNode->lexVal.s);
		}

		bool success = program->types->registerType(type);
		if(!success) {
			std::stringstream s;
			s << "Redefinition of structure " << type->name;
			throw TypeError(node, s.str());
		}
	}

	/*!
	 * \brief Add a class to the type list
	 * \param node Node describing structure
	 * \param program Program to add to
	 */
	void ProgramGenerator::addClass(Node *node, Program *program)
	{
		TypeStruct *type = new TypeStruct(Type::TypeClass, node->lexVal.s);
		Node *members = node->children[0];
		for(unsigned int i=0; i<members->children.size(); i++) {
			Node *memberNode = members->children[i];
			switch(memberNode->nodeType) {
				case Node::NodeTypeVarDecl:
				{
					Type *memberType = createType(memberNode->children[0], program->types);
					type->addMember(memberType, memberNode->lexVal.s);
					break;
				}

				case Node::NodeTypeProcedureDef:
				{
					std::stringstream s;
					s << type->name << "." << memberNode->lexVal.s;
					Procedure *procedure = generateProcedure(memberNode, program, s.str());
					type->addMember(procedure->type, memberNode->lexVal.s);
					break;
				}
			}
		}

		bool success = program->types->registerType(type);
		if(!success) {
			std::stringstream s;
			s << "Redefinition of class " << type->name;
			throw TypeError(node, s.str());
		}
	}

	/*!
	 * \brief Search for a type named by a node
	 * \param node Node describing type
	 * \return Type
	 */
	Type *ProgramGenerator::createType(Node *node, Types *types)
	{
		Type *type = 0;
		if(node->nodeType == Node::NodeTypeArray) {
			type = createType(node->children[0], types);
			if(Type::equals(type, Types::intrinsic(Types::Void))) {
				throw TypeError(node, "Cannot declare array of voids");
			}
			type = new TypeArray(type);
		} else {
			std::string name = node->lexVal.s;
			type = types->findType(name);
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
				node->type = Types::intrinsic(Types::Void);
				break;

			case Node::NodeTypePrint:
				checkChildren(node, context);

				node->children[0] = coerce(node->children[0], Types::intrinsic(Types::String));
				node->type = Types::intrinsic(Types::Void);
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

					// Coerce call arguments to proper types
					for(unsigned int i=0; i<argumentsNode->children.size(); i++) {
						argumentsNode->children[i] = coerce(argumentsNode->children[i], procedureType->argumentTypes[i]);
					}

					// Checks succeeded, assign return type to the call node
					node->type = procedureType->returnType;
					break;
				}

			case Node::NodeTypeVarDecl:
				{
					// Add the declared variable to the current scope
					Type *type = createType(node->children[0], context.types);
					if(Type::equals(type, Types::intrinsic(Types::Void))) {
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

					node->children[1] = coerce(node->children[1], node->children[0]->type);

					Node *lhs = node->children[0];
					Node *rhs = node->children[1];
					switch(lhs->nodeType) {
						case Node::NodeTypeId:
						case Node::NodeTypeVarDecl:
						case Node::NodeTypeArray:
						case Node::NodeTypeMember:
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
					if(!Type::equals(node->children[0]->type, Types::intrinsic(Types::Bool))) {
						throw TypeError(node, "Type mismatch");
					}

					node->type = Types::intrinsic(Types::Void);
					break;
				}

			case Node::NodeTypeWhile:
				{
					Context childContext = context;
					childContext.scope = new Scope(context.scope);
					childContext.inLoop = true;

					checkChildren(node, childContext);
					if(!Type::equals(node->children[0]->type, Types::intrinsic(Types::Bool))) {
						throw TypeError(node, "Type mismatch");
					}

					node->type = Types::intrinsic(Types::Void);
					break;
				}

			case Node::NodeTypeFor:
				{
					Context childContext = context;
					childContext.scope = new Scope(context.scope);
					childContext.inLoop = true;

					checkChildren(node, childContext);
					if(!Type::equals(node->children[1]->type, Types::intrinsic(Types::Bool))) {
						throw TypeError(node, "Type mismatch");
					}

					node->type = Types::intrinsic(Types::Void);
					break;
				}

			case Node::NodeTypeCompare:
				checkChildren(node, context);

				switch(node->nodeSubtype) {
					case Node::NodeSubtypeAnd:
					case Node::NodeSubtypeOr:
						coerceChildren(node, Types::intrinsic(Types::Bool));
						break;

					default:
						{
							Types::Intrinsic intrinsics[] = { Types::Int, Types::Bool, Types::Char };
							bool found = false;
							for(int i=0; i<sizeof(intrinsics) / sizeof(intrinsics[0]); i++) {
								Type *type = Types::intrinsic(intrinsics[i]);
								if(isChildOfType(node, type)) {
									coerceChildren(node, type);
									found = true;
									break;
								}
							}

							if(!found) {
								throw TypeError(node, "Type mismatch");
							}
							break;
						}
				}

				node->type = Types::intrinsic(Types::Bool);
				break;

			case Node::NodeTypeArith:
				{
					checkChildren(node, context);

					if(node->nodeSubtype == Node::NodeSubtypeAdd && isChildOfType(node, Types::intrinsic(Types::String))) {
						coerceChildren(node, Types::intrinsic(Types::String));
						node->type = Types::intrinsic(Types::String);
					} else if(node->nodeSubtype == Node::NodeSubtypeAdd && isChildOfType(node, Types::intrinsic(Types::Char)) && isChildOfType(node, Types::intrinsic(Types::Int))) {
						node->type = Types::intrinsic(Types::Char);
					} else {
						coerceChildren(node, Types::intrinsic(Types::Int));
						node->type = Types::intrinsic(Types::Int);
					}
					break;
				}

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
				if(Type::equals(context.procedure->type->returnType, Types::intrinsic(Types::Void))) {
					throw TypeError(node, "Return statement in void procedure");
				}

				checkType(node->children[0], context);

				// Coerce the return argument to the procedure's return type
				node->children[0] = coerce(node->children[0], context.procedure->type->returnType);
				node->type = Types::intrinsic(Types::Void);
				break;

			case Node::NodeTypeNew:
				{
					Node *typeNode = node->children[0];
					typeNode->type = createType(typeNode, context.types);
					node->type = typeNode->type;

					// Check array size argument
					if(typeNode->nodeType == Node::NodeTypeArray && typeNode->children.size() > 1) {
						checkType(typeNode->children[1], context);
						if(!Type::equals(typeNode->children[1]->type, Types::intrinsic(Types::Int))) {
							throw TypeError(typeNode->children[1], "Non-integral type used for array size");
						}
					}

					// Check constructor arguments if present
					if(node->children.size() > 1) {
						Node *argsNode = node->children[1];
						checkType(argsNode, context);

						// Only strings can have a constructor argument
						if(!Type::equals(typeNode->type, Types::intrinsic(Types::String))) {
							throw TypeError(node, "Constructor argument passed to non-string");
						}

						if(argsNode->children.size() != 1) {
							throw TypeError(node->children[1], "Improper number of arguments to constructor");
						}

						argsNode->children[0] = coerce(argsNode->children[0], Types::intrinsic(Types::Int));
					} else if(Type::equals(typeNode->type, Types::intrinsic(Types::String))) {
						throw TypeError(typeNode, "No arguments passed to string allocation");
					}

					break;
				}

			case Node::NodeTypeArray:
				{
					checkChildren(node, context);
					Node *baseNode = node->children[0];
					Node *subscriptNode = node->children[1];

					// Check array subscript
					if(!Type::equals(subscriptNode->type, Types::intrinsic(Types::Int))) {
						throw TypeError(subscriptNode, "Non-integral subscript");
					}

					if(Type::equals(baseNode->type, Types::intrinsic(Types::String))) {
						// Indexing a string produces a character
						node->type = Types::intrinsic(Types::Char);
					} else if(baseNode->type->type == Type::TypeArray) {
						// Indexing an array produces its base type
						TypeArray *typeArray = (TypeArray*)baseNode->type;
						node->type = typeArray->baseType;
					} else {
						throw TypeError(baseNode, "Attempt to take subscript of illegal object");
					}

					break;
				}

			case Node::NodeTypeBreak:
				if(!context.inLoop) {
					throw TypeError(node, "Break statement outside of loop");
				}
				node->type = Types::intrinsic(Types::Void);
				break;

			case Node::NodeTypeContinue:
				if(!context.inLoop) {
					throw TypeError(node, "Continue statement outside of loop");
				}
				node->type = Types::intrinsic(Types::Void);
				break;

			case Node::NodeTypeMember:
			{
				checkType(node->children[0], context);
				Node *base = node->children[0];

				if(base->type->type == Type::TypeStruct || base->type->type == Type::TypeClass) {
					// Check structure field
					TypeStruct *typeStruct = (TypeStruct*)base->type;
					TypeStruct::Member *member = typeStruct->findMember(node->lexVal.s);
					if(member) {
						node->type = member->type;
					} else {
						std::stringstream s;
						s << "No member named \'" << node->lexVal.s << "\'";
						throw TypeError(node, s.str());
					}
				} else {
					throw TypeError(node, "Attempt to take member of non-structure");
				}
			}
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