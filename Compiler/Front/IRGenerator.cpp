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
	IR::Program *IRGenerator::generate(Program *program)
	{
		IR::Program *irProgram = new IR::Program;

		// Create an IR procedure for each procedure definition node
		for(unsigned int i=0; i<program->procedures.size(); i++) {
			Procedure *procedure = program->procedures[i];
			IR::Procedure *irProcedure = new IR::Procedure(procedure->name);

			// Emit procedure prologue
			irProcedure->emit(new IR::EntryThreeAddr(IR::Entry::TypePrologue));

			// Add local variables to the procedure
			const std::vector<Symbol*> locals = procedure->scope->symbols();
			std::set<std::string> names;
			for(unsigned int j=0; j<locals.size(); j++) {
				std::string name;
				for(int idx=0;; idx++) {
					std::stringstream s;
					s << locals[j]->name;
					if(idx > 0) {
						s << "." << idx;
					}
					name = s.str();
					if(names.find(name) == names.end()) {
						names.insert(name);
						break;
					}
				}
				IR::Symbol *irSymbol = new IR::Symbol(name, locals[j]->type->valueSize, locals[j]);
				irProcedure->addSymbol(irSymbol);

				for(unsigned int k=0; k<procedure->arguments.size(); k++) {
					if(procedure->arguments[k] == locals[j]) {
						int arg = k;
						if(procedure->object) {
							arg++;
						}

						// If this symbol is an argument, emit an argument load instruction
						irProcedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadArg, irSymbol, 0, 0, arg));
					}
				}
			}

			// Construct context
			Context context;
			context.procedure = irProcedure;
			context.breakTarget = 0;
			context.continueTarget = 0;
			if(procedure->object) {
				context.object = irProcedure->findSymbol(procedure->object);
				irProcedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadArg, context.object, 0, 0, 0));
				TypeStruct *classType = procedure->scope->parent()->classType();
				if(procedure->name == classType->name + "." + classType->name && classType->vtableSize > 0) {
					IR::Symbol *vtable = irProcedure->newTemp(4);
					irProcedure->emit(new IR::EntryString(IR::Entry::TypeLoadAddress, vtable, classType->name + "$$vtable"));
					irProcedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreMem, vtable, context.object, 0));
				}
			} else {
				context.object = 0;
			}

			// Emit procedure body
			processNode(procedure->body, context);

			// If the procedure's return type is void, emit an return statement 
			if(Type::equals(procedure->type->returnType, Types::intrinsic(Types::Void))) {
				irProcedure->entries().insert(irProcedure->entries().end(), new IR::EntryThreeAddr(IR::Entry::TypeStoreRet, 0, 0));
			}

			// Emit function epilogue
			irProcedure->entries().insert(irProcedure->entries().end(), new IR::EntryThreeAddr(IR::Entry::TypeEpilogue));

			// Add procedure to procedure list
			irProgram->addProcedure(irProcedure);
		}

		for(unsigned int i=0; i<program->types->types().size(); i++) {
			const Type *type = program->types->types()[i];
			if(type->type == Type::TypeClass && ((TypeStruct*)type)->vtableSize > 0) {
				TypeStruct *typeStruct = (TypeStruct*)type;
				std::string name = typeStruct->name + "$$vtable";
				std::vector<std::string> vtable(typeStruct->vtableSize);
				while(typeStruct) {
					for(unsigned int j=0; j<typeStruct->members.size(); j++) {
						if(typeStruct->members[j].virtualFunction && vtable[typeStruct->members[j].offset] == "") {
							vtable[typeStruct->members[j].offset] = typeStruct->name + "." + typeStruct->members[j].name;
						}
					}
					typeStruct = typeStruct->parent;
				}

				IR::Data *irData = new IR::Data(name);
				for(unsigned int j=0; j<vtable.size(); j++) {
					irData->entries().push_back(new IR::EntryCall(IR::Entry::TypeFunctionAddr, vtable[j]));
				}
				irProgram->addData(irData);
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
		IR::Procedure *procedure = context.procedure;

		switch(node->nodeType) {
			case Node::NodeTypeList:
				// Process each item in the list
				for(unsigned int i=0; i<node->children.size(); i++) {
					processNode(node->children[i], context);
				}
				break;

			case Node::NodeTypePrint:
				// Process RHS value
				rhs = processRValue(node->children[0], context);

				// Emit print instruction
				procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypePrint, 0, rhs));
				break;

			case Node::NodeTypeVarDecl:
				// No work to do, symbol was already added when creating the procedure
				break;

			case Node::NodeTypeIf:
				// Process statement predicate
				lhs = processRValue(node->children[0], context);
				if(node->children.size() == 2) {
					IR::EntryLabel *trueLabel = procedure->newLabel();
					IR::EntryLabel *nextLabel = procedure->newLabel();

					// Emit conditional jump instruction to either true label or label after statement
					procedure->emit(new IR::EntryCJump(lhs, trueLabel, nextLabel));

					// Process true body
					procedure->emit(trueLabel);
					processNode(node->children[1], context);

					// Emit label following statement
					procedure->emit(nextLabel);
				} else {
					IR::EntryLabel *trueLabel = procedure->newLabel();
					IR::EntryLabel *falseLabel = procedure->newLabel();
					IR::EntryLabel *nextLabel = procedure->newLabel();

					// Emit conditional jump instruction to either true label or false label
					procedure->emit(new IR::EntryCJump(lhs, trueLabel, falseLabel));

					// Process true body
					procedure->emit(trueLabel);
					processNode(node->children[1], context);
					procedure->emit(new IR::EntryJump(nextLabel));

					// Process false body
					procedure->emit(falseLabel);
					processNode(node->children[2], context);

					// Emit label following statement
					procedure->emit(nextLabel);
				}
				break;

			case Node::NodeTypeWhile:
				{
					IR::EntryLabel *testLabel = procedure->newLabel();
					IR::EntryLabel *mainLabel = procedure->newLabel();
					IR::EntryLabel *nextLabel = procedure->newLabel();

					// Emit test of predicate and conditional jump
					procedure->emit(testLabel);
					lhs = processRValue(node->children[0], context);
					procedure->emit(new IR::EntryCJump(lhs, mainLabel, nextLabel));

					// Construct child context
					Context childContext = context;
					childContext.breakTarget = nextLabel;
					childContext.continueTarget = testLabel;

					// Emit body label
					procedure->emit(mainLabel);
					processNode(node->children[1], childContext);
					procedure->emit(new IR::EntryJump(testLabel));

					// Emit label following statement
					procedure->emit(nextLabel);
					break;
				}

			case Node::NodeTypeFor:
				{
					IR::EntryLabel *testLabel = procedure->newLabel();
					IR::EntryLabel *mainLabel = procedure->newLabel();
					IR::EntryLabel *postLabel = procedure->newLabel();
					IR::EntryLabel *nextLabel = procedure->newLabel();

					// Emit initialization
					processNode(node->children[0], context);

					// Emit test of predicate and conditional jump
					procedure->emit(testLabel);
					lhs = processRValue(node->children[1], context);
					procedure->emit(new IR::EntryCJump(lhs, mainLabel, nextLabel));

					// Construct child context
					Context childContext = context;
					childContext.breakTarget = nextLabel;
					childContext.continueTarget = postLabel;

					// Emit body label
					procedure->emit(mainLabel);
					processNode(node->children[3], childContext);

					procedure->emit(postLabel);
					processNode(node->children[2], context);
					procedure->emit(new IR::EntryJump(testLabel));

					// Emit label following statement
					procedure->emit(nextLabel);
					break;
				}

			case Node::NodeTypeReturn:
				{
					// Emit code for return value
					rhs = processRValue(node->children[0], context);

					// Emit procedure return and jump to end block
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreRet, 0, rhs));
					procedure->emit(new IR::EntryJump(procedure->end()));

					// Emit label following statement
					IR::EntryLabel *label = procedure->newLabel();
					procedure->emit(label);
					break;
				}

			case Node::NodeTypeBreak:
				procedure->emit(new IR::EntryJump(context.breakTarget));
				break;

			case Node::NodeTypeContinue:
				procedure->emit(new IR::EntryJump(context.continueTarget));
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
		IR::Procedure *procedure = context.procedure;

		switch(node->nodeType) {
			case Node::NodeTypeConstant:
				// Construct a temporary to contain the new value
				result = procedure->newTemp(node->type->valueSize);
				if(Type::equals(node->type, Types::intrinsic(Types::String))) {
					procedure->emit(new IR::EntryString(IR::Entry::TypeLoadString, result, node->lexVal.s));
				} else {
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, result, 0, 0, node->lexVal.i));
				}
				break;

			case Node::NodeTypeId:
				if(node->symbol->scope->classType()) {
					// Emit the load from the calculated memory location
					result = procedure->newTemp(node->type->valueSize);
					Front::TypeStruct::Member *member = node->symbol->scope->classType()->findMember(node->lexVal.s);
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadMem, result, context.object, 0, member->offset));
				} else {
					// Return the already-existing variable node
					result = procedure->findSymbol(node->symbol);
				}
				break;

			case Node::NodeTypeAssign:
				{
					Node *lhs = node->children[0];
					Node *rhs = node->children[1];

					// If the LHS is a declaration, process it so that the symbol gets added
					if(lhs->nodeType == Node::NodeTypeVarDecl) {
						processNode(lhs, context);
					}

					// Emit code for R-Value of assignment
					b = processRValue(rhs, context);

					if(lhs->nodeType == Node::NodeTypeId || lhs->nodeType == Node::NodeTypeVarDecl) {
						if(lhs->symbol->scope->classType()) {
							Front::TypeStruct::Member *member = lhs->symbol->scope->classType()->findMember(lhs->lexVal.s);
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreMem, b, context.object, 0, member->offset));
						} else {
							// Locate symbol to assign into
							a = procedure->findSymbol(lhs->symbol);

							// Emit a move into the target symbol
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, a, b));
						}
					} else if(lhs->nodeType == Node::NodeTypeArray) {
						// Emit code to calculate the array's base address
						a = processRValue(lhs->children[0], context);

						// Emit code to calculate the array subscript
						IR::Symbol *subscript = processRValue(lhs->children[1], context);

						// Compute the offset into array memory based on subscript and type size
						IR::Symbol *offset = procedure->newTemp(4);
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMult, offset, subscript, 0, node->type->valueSize));

						// Emit the store into the calculated memory location
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreMem, b, a, offset));
					} else if(lhs->nodeType == Node::NodeTypeMember) {
						a = processRValue(lhs->children[0], context);

						Front::TypeStruct *typeStruct = (Front::TypeStruct*)lhs->children[0]->type;
						Front::TypeStruct::Member *member = typeStruct->findMember(lhs->lexVal.s);
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreMem, b, a, 0, member->offset));
					}

					// Return the resulting node
					result = b;
					break;
				}

			case Node::NodeTypeCall:
				{
					Node *target = node->children[0];
					std::string name;
					std::vector<IR::Symbol*> args;
					switch(target->nodeType) {
						case Node::NodeTypeId:
							if(target->symbol->scope->classType()) {
								std::stringstream s;
								s << target->symbol->scope->classType()->name << "." << target->lexVal.s;
								name = s.str();
								args.push_back(context.object);
							} else {
								name = target->lexVal.s;
							}
							break;

						case Node::NodeTypeMember:
							{
								Node *base = target->children[0];
								TypeStruct *baseType = (TypeStruct*)base->type;
								Symbol *symbol = baseType->scope->findSymbol(target->lexVal.s);

								std::stringstream s;
								s << symbol->scope->classType()->name << "." << target->lexVal.s;
								name = s.str();

								IR::Symbol *object = processRValue(base, context);
								args.push_back(object);
								break;
							}
					}

					// Emit code for each argument, building a list of resulting symbols
					for(unsigned int i=0; i<node->children[1]->children.size(); i++) {
						IR::Symbol *arg = processRValue(node->children[1]->children[i], context);
						args.push_back(arg);
					}

					// Emit argument store values for each argument
					for(unsigned int i=0; i<args.size(); i++) {
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreArg, 0, args[i], 0, i));
					}

					// Emit procedure call
					procedure->emit(new IR::EntryCall(IR::Entry::TypeCall, name));

					if(!Type::equals(node->type, Types::intrinsic(Types::Void))) {
						// Assign return value to a new temporary
						result = procedure->newTemp(node->type->valueSize);
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadRet, result));
					} else {
						result = 0;
					}

					break;
				}

			case Node::NodeTypeArith:
				{
					result = procedure->newTemp(node->type->valueSize);

					// Emit code for operator arguments
					std::vector<IR::Symbol*> arguments;
					for(unsigned int i=0; i<node->children.size(); i++) {
						arguments.push_back(processRValue(node->children[i], context));
					}

					// Emit the appropriate type of arithmetic operation
					switch(node->nodeSubtype) {
						case Node::NodeSubtypeAdd:
							if(Type::equals(node->type, Types::intrinsic(Types::String))) {
								procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreArg, 0, arguments[0], 0, 0));
								procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreArg, 0, arguments[1], 0, 1));
								procedure->emit(new IR::EntryCall(IR::Entry::TypeCall, "__string_concat"));
								procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadRet, result));
							} else {
								procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeAdd, result, arguments[0], arguments[1]));
							}
							break;

						case Node::NodeSubtypeSubtract:
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeSubtract, result, arguments[0], arguments[1]));
							break;

						case Node::NodeSubtypeMultiply:
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMult, result, arguments[0], arguments[1]));
							break;

						case Node::NodeSubtypeDivide:
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeDivide, result, arguments[0], arguments[1]));
							break;

						case Node::NodeSubtypeModulo:
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeModulo, result, arguments[0], arguments[1]));
							break;

						case Node::NodeSubtypeIncrement:
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, result, arguments[0]));
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeAdd, arguments[0], arguments[0], 0, 1));
							break;

						case Node::NodeSubtypeDecrement:
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, result, arguments[0]));
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeAdd, arguments[0], arguments[0], 0, -1));
							break;
					}
					break;
				}

			case Node::NodeTypeCompare:
				// Construct a new temporary to hold value
				result = procedure->newTemp(node->type->valueSize);

				// Emit code for operator arguments
				a = processRValue(node->children[0], context);
				b = processRValue(node->children[1], context);

				// Emit the appropriate type of comparison operation
				switch(node->nodeSubtype) {
					case Node::NodeSubtypeEqual:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeEqual, result, a, b));
						break;

					case Node::NodeSubtypeNequal:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeNequal, result, a, b));
						break;

					case Node::NodeSubtypeLessThan:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLessThan, result, a, b));
						break;

					case Node::NodeSubtypeLessThanEqual:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLessThanE, result, a, b));
						break;

					case Node::NodeSubtypeGreaterThan:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeGreaterThan, result, a, b));
						break;

					case Node::NodeSubtypeGreaterThanEqual:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeGreaterThanE, result, a, b));
						break;

					case Node::NodeSubtypeOr:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeOr, result, a, b));
						break;

					case Node::NodeSubtypeAnd:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeAnd, result, a, b));
						break;
				}
				break;

			case Node::NodeTypeNew:
				{
					Node *arg = node->children[0];

					// Construct a new temporary to hold value
					result = procedure->newTemp(arg->type->valueSize);

					IR::Symbol *size = procedure->newTemp(4);
					if(arg->nodeType == Node::NodeTypeArray && arg->children.size() == 2) {
						// Array allocation: total size is typeSize * count
						Type *type = arg->children[0]->type;
						IR::Symbol *typeSize = procedure->newTemp(4);
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, typeSize, 0, 0, type->valueSize));

						IR::Symbol *count = processRValue(arg->children[1], context);
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMult, size, typeSize, count));
					} else if(Type::equals(arg->type, Types::intrinsic(Types::String))) {
						// String allocation: total size is constructor argument value
						size = processRValue(node->children[1]->children[0], context);
					} else {
						// Single allocation: total size is type's allocSize
						Type *type = arg->type;
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, size, 0, 0, type->allocSize));
					}

					// Emit new entry
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeNew, result, size));

					// If the type is a class with a constructor, emit a call to it
					if(arg->type->type == Type::TypeClass && ((TypeStruct*)arg->type)->constructor) {
						std::vector<IR::Symbol*> args;

						// First argument is the object
						args.push_back(result);

						// Emit code for each argument, building a list of resulting symbols
						for(unsigned int i=0; i<node->children[1]->children.size(); i++) {
							IR::Symbol *arg = processRValue(node->children[1]->children[i], context);
							args.push_back(arg);
						}

						// Emit argument store values for each argument
						for(unsigned int i=0; i<args.size(); i++) {
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreArg, 0, args[i], 0, i));
						}

						// Emit procedure call
						std::string name = arg->type->name + "." + arg->type->name;
						procedure->emit(new IR::EntryCall(IR::Entry::TypeCall, name));
					}

					break;
				}

			case Node::NodeTypeArray:
				{
					result = procedure->newTemp(node->type->valueSize);

					// Emit code to calculate the array's base address
					IR::Symbol *base = processRValue(node->children[0], context);

					// Emit code to calculate the array subscript
					IR::Symbol *subscript = processRValue(node->children[1], context);

					// Compute the offset into array memory based on subscript and type size
					IR::Symbol *offset = procedure->newTemp(4);
					IR::Symbol *size = procedure->newTemp(4);

					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, size, 0, 0, node->type->valueSize));
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMult, offset, subscript, size));

					// Emit the load from the calculated memory location
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadMem, result, base, offset));
					break;
				}

			case Node::NodeTypeMember:
				{
					result = procedure->newTemp(node->type->valueSize);

					IR::Symbol *base = processRValue(node->children[0], context);

					Front::TypeStruct *typeStruct = (Front::TypeStruct*)node->children[0]->type;
					Front::TypeStruct::Member *member = typeStruct->findMember(node->lexVal.s);

					// Emit the load from the calculated memory location
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadMem, result, base, 0, member->offset));
					break;
				}

			case Node::NodeTypeCoerce:
				{
					result = procedure->newTemp(node->type->valueSize);
					IR::Symbol *source = processRValue(node->children[0], context);

					// Emit the appropriate entry for the type of conversion taking place
					if(Type::equals(node->type, Types::intrinsic(Types::String))) {
						Type *sourceType = node->children[0]->type;
						if(Type::equals(sourceType, Types::intrinsic(Types::Bool))) {
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreArg, 0, source, 0, 0));
							procedure->emit(new IR::EntryCall(IR::Entry::TypeCall, "__string_bool"));
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadRet, result));
						} else if(Type::equals(sourceType, Types::intrinsic(Types::Int))) {
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreArg, 0, source, 0, 0));
							procedure->emit(new IR::EntryCall(IR::Entry::TypeCall, "__string_int"));
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadRet, result));
						} else if(Type::equals(sourceType, Types::intrinsic(Types::Char))) {
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreArg, 0, source, 0, 0));
							procedure->emit(new IR::EntryCall(IR::Entry::TypeCall, "__string_char"));
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadRet, result));
						} else if(sourceType->type == Type::TypeArray && Type::equals(((Front::TypeArray*)sourceType)->baseType, Types::intrinsic(Types::Char))) {
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, result, source));
						}
					}

					break;
				}
		}

		return result;
	}
}
