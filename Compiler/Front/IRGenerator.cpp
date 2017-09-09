#include "Front/IRGenerator.h"

#include "Front/Node.h"
#include "Front/Type.h"

#include "IR/Program.h"
#include "IR/Procedure.h"
#include "IR/Entry.h"

#include <set>
#include <sstream>

namespace Front {
	/*!
	 * \brief Generate an IR program
	 * \param tree Syntax tree to process
	 * \return Generated IR program
	 */
	std::unique_ptr<IR::Program> IRGenerator::generate(const Program &program)
	{
		std::unique_ptr<IR::Program> irProgram = std::make_unique<IR::Program>();

		// Create an IR procedure for each procedure definition node
		for(const std::unique_ptr<Procedure> &procedure : program.procedures) {
			std::unique_ptr<IR::Procedure> irProcedure = std::make_unique<IR::Procedure>(procedure->name);

			// Emit procedure prologue
			irProcedure->emit(new IR::EntryThreeAddr(IR::Entry::Type::Prologue));

			// Add local variables to the procedure
			std::set<std::string> names;
			for(Symbol *local : procedure->scope->allSymbols()) {
				std::string name;
				for(int idx=0;; idx++) {
					std::stringstream s;
					s << local->name;
					if(idx > 0) {
						s << "." << idx;
					}
					name = s.str();
					if(names.find(name) == names.end()) {
						names.insert(name);
						break;
					}
				}
				IR::Symbol *irSymbol = new IR::Symbol(name, local->type->valueSize, local);
				irProcedure->addSymbol(irSymbol);

				int arg = 0;
				if(procedure->object) {
					arg++;
				}

				for(Symbol *argument : procedure->arguments) {
					if(argument == local) {
						// If this symbol is an argument, emit an argument load instruction
						irProcedure->emit(new IR::EntryThreeAddr(IR::Entry::Type::LoadArg, irSymbol, 0, 0, arg));
					}
					arg++;
				}
			}

			// Construct context
			Context context{ *irProcedure, 0, 0 };
			if(procedure->object) {
				context.object = irProcedure->findSymbol(procedure->object);
				irProcedure->emit(new IR::EntryThreeAddr(IR::Entry::Type::LoadArg, context.object, 0, 0, 0));
				std::shared_ptr<TypeStruct> classType = procedure->scope->parent()->classType();
				if(procedure->name == classType->name + "." + classType->name && classType->vtableSize > 0) {
					IR::Symbol *vtable = irProcedure->newTemp(4);
					irProcedure->emit(new IR::EntryString(IR::Entry::Type::LoadAddress, vtable, classType->name + "$$vtable"));
					irProcedure->emit(new IR::EntryThreeAddr(IR::Entry::Type::StoreMem, vtable, context.object, 0, classType->vtableOffset));
				}
			} else {
				context.object = 0;
			}

			// Emit procedure body
			processNode(procedure->body, context);

			// If the procedure's return type is void, emit an return statement 
			if(Type::equals(*procedure->type->returnType, *Types::intrinsic(Types::Void))) {
				irProcedure->entries().insert(irProcedure->entries().end(), new IR::EntryThreeAddr(IR::Entry::Type::StoreRet, 0, 0));
			}

			// Emit function epilogue
			irProcedure->entries().insert(irProcedure->entries().end(), new IR::EntryThreeAddr(IR::Entry::Type::Epilogue));

			// Add procedure to procedure list
			irProgram->addProcedure(std::move(irProcedure));
		}

		for(unsigned int i=0; i<program.types->types().size(); i++) {
			std::shared_ptr<Type> type = program.types->types()[i];
			if(type->kind == Type::Kind::Class && (std::static_pointer_cast<TypeStruct>(type))->vtableSize > 0) {
				std::shared_ptr<TypeStruct> typeStruct = std::static_pointer_cast<TypeStruct>(type);
				std::string name = typeStruct->name + "$$vtable";
				std::vector<std::string> vtable(typeStruct->vtableSize);
				while(typeStruct) {
					for(TypeStruct::Member &member : typeStruct->members) {
						if((member.qualifiers & TypeStruct::Member::QualifierVirtual) && vtable[member.offset] == "") {
							vtable[member.offset] = typeStruct->name + "." + member.name;
						}
					}
					typeStruct = typeStruct->parent;
				}

				std::unique_ptr<IR::Data> irData = std::make_unique<IR::Data>(name);
				for(std::string &target : vtable) {
					irData->entries().push_back(new IR::EntryCall(IR::Entry::Type::FunctionAddr, target));
				}
				irProgram->addData(std::move(irData));
			}
		}

		return irProgram;
	}

	/*!
	 * \brief Process a node in the syntax tree
	 * \param node Node to process
	 * \param context Context object
	 */
	void IRGenerator::processNode(Node *node, Context &context)
	{
		IR::Symbol *lhs, *rhs;
		IR::Procedure &procedure = context.procedure;

		switch(node->nodeType) {
			case Node::Type::List:
				// Process each item in the list
				for(Node *child : node->children) {
					processNode(child, context);
				}
				break;

			case Node::Type::VarDecl:
				// No work to do, symbol was already added when creating the procedure
				break;

			case Node::Type::If:
				// Process statement predicate
				lhs = processRValue(node->children[0], context);
				if(node->children.size() == 2) {
					IR::EntryLabel *trueLabel = procedure.newLabel();
					IR::EntryLabel *nextLabel = procedure.newLabel();

					// Emit conditional jump instruction to either true label or label after statement
					procedure.emit(new IR::EntryCJump(lhs, trueLabel, nextLabel));

					// Process true body
					procedure.emit(trueLabel);
					processNode(node->children[1], context);

					// Emit label following statement
					procedure.emit(nextLabel);
				} else {
					IR::EntryLabel *trueLabel = procedure.newLabel();
					IR::EntryLabel *falseLabel = procedure.newLabel();
					IR::EntryLabel *nextLabel = procedure.newLabel();

					// Emit conditional jump instruction to either true label or false label
					procedure.emit(new IR::EntryCJump(lhs, trueLabel, falseLabel));

					// Process true body
					procedure.emit(trueLabel);
					processNode(node->children[1], context);
					procedure.emit(new IR::EntryJump(nextLabel));

					// Process false body
					procedure.emit(falseLabel);
					processNode(node->children[2], context);

					// Emit label following statement
					procedure.emit(nextLabel);
				}
				break;

			case Node::Type::While:
				{
					IR::EntryLabel *testLabel = procedure.newLabel();
					IR::EntryLabel *mainLabel = procedure.newLabel();
					IR::EntryLabel *nextLabel = procedure.newLabel();

					// Emit test of predicate and conditional jump
					procedure.emit(testLabel);
					lhs = processRValue(node->children[0], context);
					procedure.emit(new IR::EntryCJump(lhs, mainLabel, nextLabel));

					// Construct child context
					Context childContext = context;
					childContext.breakTarget = nextLabel;
					childContext.continueTarget = testLabel;

					// Emit body label
					procedure.emit(mainLabel);
					processNode(node->children[1], childContext);
					procedure.emit(new IR::EntryJump(testLabel));

					// Emit label following statement
					procedure.emit(nextLabel);
					break;
				}

			case Node::Type::For:
				{
					IR::EntryLabel *testLabel = procedure.newLabel();
					IR::EntryLabel *mainLabel = procedure.newLabel();
					IR::EntryLabel *postLabel = procedure.newLabel();
					IR::EntryLabel *nextLabel = procedure.newLabel();

					// Emit initialization
					processNode(node->children[0], context);

					// Emit test of predicate and conditional jump
					procedure.emit(testLabel);
					lhs = processRValue(node->children[1], context);
					procedure.emit(new IR::EntryCJump(lhs, mainLabel, nextLabel));

					// Construct child context
					Context childContext = context;
					childContext.breakTarget = nextLabel;
					childContext.continueTarget = postLabel;

					// Emit body label
					procedure.emit(mainLabel);
					processNode(node->children[3], childContext);

					procedure.emit(postLabel);
					processNode(node->children[2], context);
					procedure.emit(new IR::EntryJump(testLabel));

					// Emit label following statement
					procedure.emit(nextLabel);
					break;
				}

			case Node::Type::Return:
				{
					// Emit code for return value
					rhs = processRValue(node->children[0], context);

					// Emit procedure return and jump to end block
					procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::StoreRet, 0, rhs));
					procedure.emit(new IR::EntryJump(procedure.end()));

					// Emit label following statement
					IR::EntryLabel *label = procedure.newLabel();
					procedure.emit(label);
					break;
				}

			case Node::Type::Break:
				procedure.emit(new IR::EntryJump(context.breakTarget));
				break;

			case Node::Type::Continue:
				procedure.emit(new IR::EntryJump(context.continueTarget));
				break;

			default:
				processRValue(node, context);
				break;
		}
	}

	/*!
	 * \brief Process an R-Value expression
	 * \param node Tree node to process
	 * \param context Context object
	 * \return Symbol containing value
	 */
	IR::Symbol *IRGenerator::processRValue(Node *node, Context &context)
	{
		IR::Symbol *result;
		IR::Symbol *a, *b;
		IR::Procedure &procedure = context.procedure;

		switch(node->nodeType) {
			case Node::Type::Constant:
				// Construct a temporary to contain the new value
				result = procedure.newTemp(node->type->valueSize);
				if(Type::equals(*node->type, *Types::intrinsic(Types::String))) {
					procedure.emit(new IR::EntryString(IR::Entry::Type::LoadString, result, node->lexVal.s));
				} else {
					procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Move, result, 0, 0, node->lexVal.i));
				}
				break;

			case Node::Type::Id:
				if(node->symbol->scope->classType()) {
					// Emit the load from the calculated memory location
					result = procedure.newTemp(node->type->valueSize);
					Front::TypeStruct::Member *member = node->symbol->scope->classType()->findMember(node->lexVal.s);
					procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::LoadMem, result, context.object, 0, member->offset));
				} else {
					// Return the already-existing variable node
					result = procedure.findSymbol(node->symbol);
				}
				break;

			case Node::Type::Assign:
				{
					Node *lhs = node->children[0];
					Node *rhs = node->children[1];

					// If the LHS is a declaration, process it so that the symbol gets added
					if(lhs->nodeType == Node::Type::VarDecl) {
						processNode(lhs, context);
					}

					// Emit code for R-Value of assignment
					b = processRValue(rhs, context);

					if(lhs->nodeType == Node::Type::Id || lhs->nodeType == Node::Type::VarDecl) {
						if(lhs->symbol->scope->classType()) {
							Front::TypeStruct::Member *member = lhs->symbol->scope->classType()->findMember(lhs->lexVal.s);
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::StoreMem, b, context.object, 0, member->offset));
						} else {
							// Locate symbol to assign into
							a = procedure.findSymbol(lhs->symbol);

							// Emit a move into the target symbol
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Move, a, b));
						}
					} else if(lhs->nodeType == Node::Type::Array) {
						// Emit code to calculate the array's base address
						a = processRValue(lhs->children[0], context);

						// Emit code to calculate the array subscript
						IR::Symbol *subscript = processRValue(lhs->children[1], context);

						// Compute the offset into array memory based on subscript and type size
						IR::Symbol *offset = procedure.newTemp(4);
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Mult, offset, subscript, 0, node->type->valueSize));

						// Emit the store into the calculated memory location
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::StoreMem, b, a, offset));
					} else if(lhs->nodeType == Node::Type::Member) {
						a = processRValue(lhs->children[0], context);

						std::shared_ptr<Front::TypeStruct> typeStruct = std::static_pointer_cast<Front::TypeStruct>(lhs->children[0]->type);
						Front::TypeStruct::Member *member = typeStruct->findMember(lhs->lexVal.s);
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::StoreMem, b, a, 0, member->offset));
					}

					// Return the resulting node
					result = b;
					break;
				}

			case Node::Type::Call:
				{
					Node *target = node->children[0];
					std::shared_ptr<Front::TypeStruct> classType;
					IR::Symbol *object;
					std::string name;
					IR::Entry *callEntry;
					std::vector<IR::Symbol*> args;

					// Determine the target of the call, and the class type and object if it is
					// a member function call
					switch(target->nodeType) {
						case Node::Type::Id:
							classType = target->symbol->scope->classType();
							object = context.object;
							name = target->lexVal.s;
							break;

						case Node::Type::Member:
							{
								Node *base = target->children[0];
								classType = std::static_pointer_cast<TypeStruct>(base->type);
								name = target->lexVal.s;

								if(base->symbol) {
									object = processRValue(base, context);
								}

								break;
							}
					}

					// Construct the call entry
					if(classType) {
						Front::TypeStruct::Member *member = classType->findMember(name);
						if(member->qualifiers & TypeStruct::Member::QualifierVirtual) {
							IR::Symbol *vtable = procedure.newTemp(4);
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::LoadMem, vtable, object, 0, classType->vtableOffset));
							IR::Symbol *callTarget = procedure.newTemp(4);
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::LoadMem, callTarget, vtable, 0, member->offset * 4));
							callEntry = new IR::EntryThreeAddr(IR::Entry::Type::CallIndirect, 0, callTarget);
						} else {
							std::stringstream s;
							s << classType->name << "." << name;
							std::string name = s.str();
							callEntry = new IR::EntryCall(IR::Entry::Type::Call, name);
						}

						if(!(member->qualifiers & TypeStruct::Member::QualifierStatic)) {
							// Add the object as the first parameter
							args.push_back(object);
						}
					} else {
						callEntry = new IR::EntryCall(IR::Entry::Type::Call, name);
					}

					// Emit code for each argument, building a list of resulting symbols
					for(Node *argumentNode : node->children[1]->children) {
						IR::Symbol *arg = processRValue(argumentNode, context);
						args.push_back(arg);
					}

					// Emit argument store values for each argument
					int argIndex = 0;
					for(IR::Symbol *arg : args) {
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::StoreArg, 0, arg, 0, argIndex));
						argIndex++;
					}

					// Emit procedure call
					procedure.emit(callEntry);

					if(!Type::equals(*node->type, *Types::intrinsic(Types::Void))) {
						// Assign return value to a new temporary
						result = procedure.newTemp(node->type->valueSize);
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::LoadRet, result));
					} else {
						result = 0;
					}

					break;
				}

			case Node::Type::Arith:
				{
					result = procedure.newTemp(node->type->valueSize);

					// Emit code for operator arguments
					std::vector<IR::Symbol*> arguments;
					for(Node *child : node->children) {
						arguments.push_back(processRValue(child, context));
					}

					// Emit the appropriate type of arithmetic operation
					switch(node->nodeSubtype) {
					case Node::Subtype::Add:
							if(Type::equals(*node->type, *Types::intrinsic(Types::String))) {
								procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::StoreArg, 0, arguments[0], 0, 0));
								procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::StoreArg, 0, arguments[1], 0, 1));
								procedure.emit(new IR::EntryCall(IR::Entry::Type::Call, "__string_concat"));
								procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::LoadRet, result));
							} else {
								procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Add, result, arguments[0], arguments[1]));
							}
							break;

						case Node::Subtype::Subtract:
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Subtract, result, arguments[0], arguments[1]));
							break;

						case Node::Subtype::Multiply:
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Mult, result, arguments[0], arguments[1]));
							break;

						case Node::Subtype::Divide:
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Divide, result, arguments[0], arguments[1]));
							break;

						case Node::Subtype::Modulo:
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Modulo, result, arguments[0], arguments[1]));
							break;

						case Node::Subtype::Increment:
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Move, result, arguments[0]));
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Add, arguments[0], arguments[0], 0, 1));
							break;

						case Node::Subtype::Decrement:
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Move, result, arguments[0]));
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Add, arguments[0], arguments[0], 0, -1));
							break;
					}
					break;
				}

			case Node::Type::Compare:
				// Construct a new temporary to hold value
				result = procedure.newTemp(node->type->valueSize);

				// Emit code for operator arguments
				a = processRValue(node->children[0], context);
				b = processRValue(node->children[1], context);

				// Emit the appropriate type of comparison operation
				switch(node->nodeSubtype) {
					case Node::Subtype::Equal:
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Equal, result, a, b));
						break;

					case Node::Subtype::Nequal:
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Nequal, result, a, b));
						break;

					case Node::Subtype::LessThan:
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::LessThan, result, a, b));
						break;

					case Node::Subtype::LessThanEqual:
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::LessThanE, result, a, b));
						break;

					case Node::Subtype::GreaterThan:
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::GreaterThan, result, a, b));
						break;

					case Node::Subtype::GreaterThanEqual:
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::GreaterThanE, result, a, b));
						break;

					case Node::Subtype::Or:
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Or, result, a, b));
						break;

					case Node::Subtype::And:
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::And, result, a, b));
						break;
				}
				break;

			case Node::Type::New:
				{
					Node *arg = node->children[0];

					// Construct a new temporary to hold value
					result = procedure.newTemp(arg->type->valueSize);

					IR::Symbol *size = procedure.newTemp(4);
					if(arg->nodeType == Node::Type::Array && arg->children.size() == 2) {
						// Array allocation: total size is typeSize * count
						std::shared_ptr<Type> &type = arg->children[0]->type;
						IR::Symbol *typeSize = procedure.newTemp(4);
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Move, typeSize, 0, 0, type->valueSize));

						IR::Symbol *count = processRValue(arg->children[1], context);
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Mult, size, typeSize, count));
					} else if(Type::equals(*arg->type, *Types::intrinsic(Types::String))) {
						// String allocation: total size is constructor argument value
						size = processRValue(node->children[1]->children[0], context);
					} else {
						// Single allocation: total size is type's allocSize
						std::shared_ptr<Type> &type = arg->type;
						procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Move, size, 0, 0, type->allocSize));
					}

					// Emit new entry
					procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::New, result, size));

					// If the type is a class with a constructor, emit a call to it
					if(arg->type->kind == Type::Kind::Class && std::static_pointer_cast<TypeStruct>(arg->type)->constructor) {
						std::vector<IR::Symbol*> args;

						// First argument is the object
						args.push_back(result);

						// Emit code for each argument, building a list of resulting symbols
						for(Node *child : node->children[1]->children) {
							IR::Symbol *arg = processRValue(child, context);
							args.push_back(arg);
						}

						// Emit argument store values for each argument
						int argIndex = 0;
						for(IR::Symbol *arg : args) {
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::StoreArg, 0, arg, 0, argIndex));
							argIndex++;
						}

						// Emit procedure call
						std::string name = arg->type->name + "." + arg->type->name;
						procedure.emit(new IR::EntryCall(IR::Entry::Type::Call, name));
					}

					break;
				}

			case Node::Type::Array:
				{
					result = procedure.newTemp(node->type->valueSize);

					// Emit code to calculate the array's base address
					IR::Symbol *base = processRValue(node->children[0], context);

					// Emit code to calculate the array subscript
					IR::Symbol *subscript = processRValue(node->children[1], context);

					// Compute the offset into array memory based on subscript and type size
					IR::Symbol *offset = procedure.newTemp(4);
					IR::Symbol *size = procedure.newTemp(4);

					procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Move, size, 0, 0, node->type->valueSize));
					procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Mult, offset, subscript, size));

					// Emit the load from the calculated memory location
					procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::LoadMem, result, base, offset));
					break;
				}

			case Node::Type::Member:
				{
					result = procedure.newTemp(node->type->valueSize);

					IR::Symbol *base = processRValue(node->children[0], context);

					std::shared_ptr<Front::TypeStruct> typeStruct = std::static_pointer_cast<Front::TypeStruct>(node->children[0]->type);
					Front::TypeStruct::Member *member = typeStruct->findMember(node->lexVal.s);

					// Emit the load from the calculated memory location
					procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::LoadMem, result, base, 0, member->offset));
					break;
				}

			case Node::Type::Coerce:
				{
					IR::Symbol *source = processRValue(node->children[0], context);

					// Emit the appropriate entry for the type of conversion taking place
					if(Type::equals(*node->type, *Types::intrinsic(Types::String))) {
						result = procedure.newTemp(node->type->valueSize);

						std::shared_ptr<Type> &sourceType = node->children[0]->type;
						if(Type::equals(*sourceType, *Types::intrinsic(Types::Bool))) {
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::StoreArg, 0, source, 0, 0));
							procedure.emit(new IR::EntryCall(IR::Entry::Type::Call, "__string_bool"));
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::LoadRet, result));
						} else if(Type::equals(*sourceType, *Types::intrinsic(Types::Int))) {
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::StoreArg, 0, source, 0, 0));
							procedure.emit(new IR::EntryCall(IR::Entry::Type::Call, "__string_int"));
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::LoadRet, result));
						} else if(Type::equals(*sourceType, *Types::intrinsic(Types::Char))) {
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::StoreArg, 0, source, 0, 0));
							procedure.emit(new IR::EntryCall(IR::Entry::Type::Call, "__string_char"));
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::LoadRet, result));
						} else if(sourceType->kind == Type::Kind::Array && Type::equals(*std::static_pointer_cast<Front::TypeArray>(sourceType)->baseType, *Types::intrinsic(Types::Char))) {
							procedure.emit(new IR::EntryThreeAddr(IR::Entry::Type::Move, result, source));
						}
					} else {
						result = source;
					}

					break;
				}
		}

		return result;
	}
}
