#include "Front/ProgramGenerator.h"

#include <set>
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
	ProgramGenerator::ProgramGenerator(Node *tree, std::unique_ptr<Types> types, std::unique_ptr<Scope> scope)
	{
		mTree = tree;
		mTypes = std::move(types);
		mScope = std::move(scope);
	}

	/*!
	 * \brief Generate a program
	 * \return Program structure
	 */
	std::unique_ptr<Program> ProgramGenerator::generate()
	{
		try {
			// Create the root program object
			std::unique_ptr<Program> program = std::make_unique<Program>();
			program->types = std::move(mTypes);
			program->scope = std::move(mScope);

			// Iterate through the tree's procedure definitions
			for(Node *node : mTree->children) {
				switch(node->nodeType) {
					case Node::Type::ProcedureDef:
						addProcedure(node, *program, program->scope.get(), false);
						break;

					case Node::Type::ClassDef:
						{
							std::shared_ptr<TypeStruct> typeStruct = std::static_pointer_cast<TypeStruct>(program->types->findType(node->lexVal.s));
							Node *members = node->children[node->children.size() - 1];
							for(Node *child : members->children) {
								Node *qualifiersNode = child->children[0];
								Node *memberNode = child->children[1];
								TypeStruct::Member *member = typeStruct->findMember(memberNode->lexVal.s);

								if(memberNode->nodeType == Node::Type::ProcedureDef) {
									if(member->qualifiers & TypeStruct::Member::QualifierNative) {
										if(memberNode->children.size() == 3) {
											throw TypeError(memberNode, "Native function cannot have body");
										}
									} else {
										if(memberNode->children.size() == 2) {
											throw TypeError(memberNode, "Non-native function requires a body");
										} else {
											bool instanceMethod = !(member->qualifiers & TypeStruct::Member::QualifierStatic);
											addProcedure(memberNode, *program, typeStruct->scope, instanceMethod);
										}
									}
								}
							}
							break;
						}
				}
			}

			// Now that all procedures have been declared, type check the procedure bodies
			for(std::unique_ptr<Procedure> &procedure : program->procedures) {
				// Create a context for the procedure
				Context context{ *procedure, program->types.get(), procedure->scope, false };

				// Type check the body of the procedure
				checkType(procedure->body, context);
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
	Node *coerce(Node *node, std::shared_ptr<Type> &type)
	{
		// If node is already of the given type, do nothing
		if(!Type::equals(*node->type, *type)) {
			bool valid = false;

			if(Type::equals(*type, *Types::intrinsic(Types::String))) {
				if(Type::equals(*node->type, *Types::intrinsic(Types::Bool)) ||
				   Type::equals(*node->type, *Types::intrinsic(Types::Int)) ||
				   Type::equals(*node->type, *Types::intrinsic(Types::Char)))	{
					valid = true;
				} else if(node->type->kind == Type::Kind::Array && Type::equals(*(std::static_pointer_cast<Front::TypeArray>(node->type)->baseType), *Types::intrinsic(Types::Char))) {
					valid = true;
				}
			} else if(type->kind == Type::Kind::Class && node->type->kind == Type::Kind::Class) {
				std::shared_ptr<TypeStruct> classType = std::static_pointer_cast<TypeStruct>(node->type);
				while(classType) {
					if(Type::equals(*classType, *type)) {
						valid = true;
						break;
					}
					classType = classType->parent;
				}
			}

			if(!valid) {
				std::stringstream s;
				s << "Cannot convert type " << node->type->name << " to " << type->name;
				throw TypeError(node, s.str());
			}

			Node *coerce = new Node();
			coerce->nodeType = Node::Type::Coerce;
			coerce->nodeSubtype = Node::Subtype::None;
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
	void coerceChildren(Node *node, std::shared_ptr<Type> &type)
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
	bool isChildOfType(Node *node, Type &type)
	{
		for(Node *child : node->children) {
			if(Type::equals(*child->type, type)) {
				return true;
			}
		}

		return false;
	}

	/*!
	 * \brief Generate a single procedure
	 * \param node Tree node for procedure definition
	 * \param program Program to add procedure to
	 * \param scope Parent scope of procedure
	 * \return New procedure
	 */
	void ProgramGenerator::addProcedure(Node *node, Program &program, Scope *scope, bool instanceMethod)
	{
		// Construct a procedure object
		std::unique_ptr<Procedure> procedure = std::make_unique<Procedure>();
		Symbol *symbol = scope->findSymbol(node->lexVal.s);
		procedure->type = std::static_pointer_cast<TypeProcedure>(symbol->type);

		// Begin populating the procedure object
		if(scope->classType()) {
			std::stringstream s;
			s << scope->classType()->name << "." << node->lexVal.s;
			procedure->name = s.str();
		} else {
			procedure->name = node->lexVal.s;
		}
		std::unique_ptr<Scope> newScope = std::make_unique<Scope>();		
		procedure->scope = newScope.get();
		scope->addChild(std::move(newScope));

		if(instanceMethod) {
			std::unique_ptr<Symbol> symbol = std::make_unique<Symbol>(scope->classType(), "this");
			procedure->object = symbol.get();
			procedure->scope->addSymbol(std::move(symbol));
		} else {
			procedure->object = 0;
		}

		// Iterate the tree's argument items
		Node *argumentList = node->children[1];
		for(unsigned int i=0; i<argumentList->children.size(); i++) {
			// Construct a symbol for the argument, and add it to the procedure's scope and argument list
			std::unique_ptr<Symbol> argument = std::make_unique<Symbol>(procedure->type->argumentTypes[i], argumentList->children[i]->lexVal.s);
			procedure->arguments.push_back(argument.get());
			procedure->scope->addSymbol(std::move(argument));
		}

		procedure->body = node->children[2];
		node->type = Types::intrinsic(Types::Void);

		// Add the procedure to the program's procedure list
		program.procedures.push_back(std::move(procedure));
	}

	/*!
	 * \brief Search for a type named by a node
	 * \param node Node describing type
	 * \return Type
	 */
	std::shared_ptr<Type> ProgramGenerator::createType(Node *node, Types *types)
	{
		std::shared_ptr<Type> type;
		if(node->nodeType == Node::Type::Array) {
			type = createType(node->children[0], types);
			if(Type::equals(*type, *Types::intrinsic(Types::Void))) {
				throw TypeError(node, "Cannot declare array of voids");
			}
			type = std::make_shared<TypeArray>(type);
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
			case Node::Type::List:
				checkChildren(node, context);
				node->type = Types::intrinsic(Types::Void);
				break;

			case Node::Type::Call:
				{
					// First, check all child nodes
					checkChildren(node, context);

					// Check that call target is a procedure type
					Node *procedureNode = node->children[0];
					if(procedureNode->type->kind != Type::Kind::Procedure) {
						throw TypeError(node, "Expression does not evaluate to a procedure");
					}

					std::shared_ptr<TypeProcedure> procedureType = std::static_pointer_cast<TypeProcedure>(procedureNode->type);
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

			case Node::Type::VarDecl:
				{
					// Add the declared variable to the current scope
					std::shared_ptr<Type> type = createType(node->children[0], context.types);
					if(Type::equals(*type, *Types::intrinsic(Types::Void))) {
						throw TypeError(node, "Cannot declare variable of void type");
					}

					std::unique_ptr<Symbol> symbol = std::make_unique<Symbol>(type, node->lexVal.s);
					node->type = type;
					node->symbol = symbol.get();
					context.scope->addSymbol(std::move(symbol));
					break;
				}

			case Node::Type::Assign:
				{
					checkChildren(node, context);

					node->children[1] = coerce(node->children[1], node->children[0]->type);

					Node *lhs = node->children[0];
					Node *rhs = node->children[1];
					switch(lhs->nodeType) {
						case Node::Type::Id:
						case Node::Type::VarDecl:
						case Node::Type::Array:
						case Node::Type::Member:
							break;
						default:
							throw TypeError(node, "Lvalue required");
					}

					if(lhs->nodeType == Node::Type::Array && Type::equals(*lhs->children[0]->type, *Types::intrinsic(Types::String))) {
						throw TypeError(node, "String elements cannot be assigned to");
					}

					node->type = rhs->type;
					break;
				}

			case Node::Type::If:
				{
					Context childContext = context;
					std::unique_ptr<Scope> scope = std::make_unique<Scope>();
					childContext.scope = scope.get();
					context.scope->addChild(std::move(scope));

					checkChildren(node, childContext);
					if(!Type::equals(*node->children[0]->type, *Types::intrinsic(Types::Bool))) {
						throw TypeError(node, "Type mismatch");
					}

					node->type = Types::intrinsic(Types::Void);
					break;
				}

			case Node::Type::While:
				{
					Context childContext = context;
					std::unique_ptr<Scope> scope = std::make_unique<Scope>();
					childContext.scope = scope.get();
					context.scope->addChild(std::move(scope));
					childContext.inLoop = true;

					checkChildren(node, childContext);
					if(!Type::equals(*node->children[0]->type, *Types::intrinsic(Types::Bool))) {
						throw TypeError(node, "Type mismatch");
					}

					node->type = Types::intrinsic(Types::Void);
					break;
				}

			case Node::Type::For:
				{
					Context childContext = context;
					std::unique_ptr<Scope> scope = std::make_unique<Scope>();
					childContext.scope = scope.get();
					context.scope->addChild(std::move(scope));
					childContext.inLoop = true;

					checkChildren(node, childContext);
					if(!Type::equals(*node->children[1]->type, *Types::intrinsic(Types::Bool))) {
						throw TypeError(node, "Type mismatch");
					}

					node->type = Types::intrinsic(Types::Void);
					break;
				}

			case Node::Type::Compare:
				checkChildren(node, context);

				switch(node->nodeSubtype) {
					case Node::Subtype::And:
					case Node::Subtype::Or:
						coerceChildren(node, Types::intrinsic(Types::Bool));
						break;

					default:
						{
							Types::Intrinsic intrinsics[] = { Types::Int, Types::Bool, Types::Char };
							bool found = false;
							for(Types::Intrinsic &intrinsic : intrinsics) {
								std::shared_ptr<Type> type = Types::intrinsic(intrinsic);
								if(isChildOfType(node, *type)) {
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

			case Node::Type::Arith:
				{
					checkChildren(node, context);

					if(node->nodeSubtype == Node::Subtype::Add && isChildOfType(node, *Types::intrinsic(Types::String))) {
						coerceChildren(node, Types::intrinsic(Types::String));
						node->type = Types::intrinsic(Types::String);
					} else if(node->nodeSubtype == Node::Subtype::Add && isChildOfType(node, *Types::intrinsic(Types::Char)) && isChildOfType(node, *Types::intrinsic(Types::Int))) {
						node->type = Types::intrinsic(Types::Char);
					} else {
						coerceChildren(node, Types::intrinsic(Types::Int));
						node->type = Types::intrinsic(Types::Int);
					}
					break;
				}

			case Node::Type::Id:
				{
					// Search for the named symbol in the current scope
					std::string name = node->lexVal.s;
					Symbol *symbol = context.scope->findSymbol(name);
					if(!symbol) {
						std::stringstream s;
						s << "Undefined variable '" << name << "'";
						throw TypeError(node, s.str());
					}

					if(symbol->scope->classType() && !context.procedure.object) {
						TypeStruct::Member *member = symbol->scope->classType()->findMember(name);
						if(!(member->qualifiers & TypeStruct::Member::QualifierStatic)) {
							throw TypeError(node, "Cannot access class member from a static method");
						}
					}

					node->type = symbol->type;
					node->symbol = symbol;
					break;
				}

			case Node::Type::Constant:
				// Type provided by parser
				break;

			case Node::Type::Return:
				if(Type::equals(*context.procedure.type->returnType, *Types::intrinsic(Types::Void))) {
					throw TypeError(node, "Return statement in void procedure");
				}

				checkType(node->children[0], context);

				// Coerce the return argument to the procedure's return type
				node->children[0] = coerce(node->children[0], context.procedure.type->returnType);
				node->type = Types::intrinsic(Types::Void);
				break;

			case Node::Type::New:
				{
					Node *typeNode = node->children[0];
					typeNode->type = createType(typeNode, context.types);
					node->type = typeNode->type;

					// Strings can't be allocated, only created from literals or coerced from char[]
					if(Type::equals(*typeNode->type, *Types::intrinsic(Types::String))) {
						throw TypeError(typeNode, "Cannot allocate string type");
					}

					// Check array size argument
					if(typeNode->nodeType == Node::Type::Array && typeNode->children.size() > 1) {
						checkType(typeNode->children[1], context);
						if(!Type::equals(*typeNode->children[1]->type, *Types::intrinsic(Types::Int))) {
							throw TypeError(typeNode->children[1], "Non-integral type used for array size");
						}
					}

					// Check constructor arguments if present
					if(node->children.size() > 1) {
						Node *argsNode = node->children[1];
						checkType(argsNode, context);

						if(typeNode->type->kind == Type::Kind::Class) {
							std::shared_ptr<TypeStruct> typeStruct = std::static_pointer_cast<TypeStruct>(typeNode->type);

							// First, check all child nodes
							checkChildren(argsNode, context);

							if(typeStruct->constructor) {
								// Check call target has correct number of parameters
								if(typeStruct->constructor->argumentTypes.size() != argsNode->children.size()) {
									throw TypeError(node, "Improper number of arguments to constructor");
								}

								// Coerce call arguments to proper types
								for(unsigned int i=0; i<argsNode->children.size(); i++) {
									argsNode->children[i] = coerce(argsNode->children[i], typeStruct->constructor->argumentTypes[i]);
								}
							} else {
								if(argsNode->children.size() != 0) {
									throw TypeError(node, "Constructor arguments passed to class with no constructor");
								}
							}
						} else {
							throw TypeError(node->children[1], "Constuctor arguments passed to non-class type");
						}
					} else if(typeNode->type->kind == Type::Kind::Class) {
						throw TypeError(node->children[1], "Missing constructor arguments");
					}

					break;
				}

			case Node::Type::Array:
				{
					checkChildren(node, context);
					Node *baseNode = node->children[0];
					Node *subscriptNode = node->children[1];

					// Check array subscript
					if(!Type::equals(*subscriptNode->type, *Types::intrinsic(Types::Int))) {
						throw TypeError(subscriptNode, "Non-integral subscript");
					}

					if(Type::equals(*baseNode->type, *Types::intrinsic(Types::String))) {
						// Indexing a string produces a character
						node->type = Types::intrinsic(Types::Char);
					} else if(baseNode->type->kind == Type::Kind::Array) {
						// Indexing an array produces its base type
						std::shared_ptr<TypeArray> typeArray = std::static_pointer_cast<TypeArray>(baseNode->type);
						node->type = typeArray->baseType;
					} else {
						throw TypeError(baseNode, "Attempt to take subscript of illegal object");
					}

					break;
				}

			case Node::Type::Break:
				if(!context.inLoop) {
					throw TypeError(node, "Break statement outside of loop");
				}
				node->type = Types::intrinsic(Types::Void);
				break;

			case Node::Type::Continue:
				if(!context.inLoop) {
					throw TypeError(node, "Continue statement outside of loop");
				}
				node->type = Types::intrinsic(Types::Void);
				break;

			case Node::Type::Member:
			{
				Node *base = node->children[0];

				bool typeName;
				std::shared_ptr<TypeStruct> baseType = std::static_pointer_cast<TypeStruct>(context.types->findType(base->lexVal.s));
				if(baseType) {
					base->type = baseType;
					typeName = true;
				} else {
					checkType(node->children[0], context);
					typeName = false;
				}

				if(base->type->kind == Type::Kind::Struct || base->type->kind == Type::Kind::Class) {
					// Check structure field
					std::shared_ptr<TypeStruct> typeStruct = std::static_pointer_cast<TypeStruct>(base->type);
					TypeStruct::Member *member = typeStruct->findMember(node->lexVal.s);
					if(member) {
						if(typeName && !(member->qualifiers & TypeStruct::Member::QualifierStatic)) {
							throw TypeError(node, "Attempt to address instance member via class name");
						}
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
		for(Node *child : node->children) {
			checkType(child, context);
		}
	}
}