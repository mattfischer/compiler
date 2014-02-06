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
			IR::Procedure *irProcedure = new IR::Procedure(procedure->name, !Type::equals(procedure->type->returnType, program->types->intrinsic(Types::Void)));

			// Emit procedure prologue
			irProcedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypePrologue, 0, 0, 0));

			// Add local variables to the procedure
			const std::vector<Symbol*> locals = procedure->locals->symbols();
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
				IR::Symbol *irSymbol = new IR::Symbol(name, locals[j]);
				irProcedure->addSymbol(irSymbol);

				for(unsigned int k=0; k<procedure->arguments.size(); k++) {
					if(procedure->arguments[k] == locals[j]) {
						// If this symbol is an argument, emit an argument load instruction
						irProcedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeLoadArg, irSymbol, 0, k));
					}
				}
			}

			// Construct context
			Context context;
			context.program = irProgram;
			context.procedure = irProcedure;
			context.breakTarget = 0;
			context.continueTarget = 0;

			// Emit procedure body
			processNode(procedure->body, context);

			// Emit function epilogue
			irProcedure->entries().insert(irProcedure->entries().end(), new IR::EntryTwoAddrImm(IR::Entry::TypeEpilogue, 0, 0, 0));

			// Add procedure to procedure list
			irProgram->addProcedure(irProcedure);
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
				result = procedure->newTemp();
				procedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeLoadImm, result, 0, node->lexVal.i));
				break;

			case Node::NodeTypeId:
				// Return the already-existing variable node
				result = procedure->findSymbol(node->symbol);
				break;

			case Node::NodeTypeAssign:
				// If the LHS is a declaration, process it so that the symbol gets added
				if(node->children[0]->nodeType == Node::NodeTypeVarDecl) {
					processNode(node->children[0], context);
				}

				// Emit code for R-Value of assignment
				b = processRValue(node->children[1], context);

				if(node->children[0]->nodeType == Node::NodeTypeId || node->children[0]->nodeType == Node::NodeTypeVarDecl) {
					// Locate symbol to assign into
					a = procedure->findSymbol(node->children[0]->symbol);

					// Emit a move into the target symbol
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, a, b));
				} else if(node->children[0]->nodeType == Node::NodeTypeArray) {
					Node *arrayNode = node->children[0];

					// Emit code to calculate the array's base address
					a = processRValue(arrayNode->children[0], context);

					// Emit code to calculate the array subscript
					IR::Symbol *subscript = processRValue(arrayNode->children[1], context);

					// Compute the offset into array memory based o subscript and type size
					IR::Symbol *offset = procedure->newTemp();
					IR::Symbol *size = procedure->newTemp();

					procedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeLoadImm, size, 0, node->type->size / 4));
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMult, offset, subscript, size));

					// Emit the store into the calculated memory location
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreMemInd, b, a, offset));
				} else if(node->children[0]->nodeType == Node::NodeTypeMember) {
					Node *memberNode = node->children[0];

					a = processRValue(memberNode->children[0], context);

					int idx;
					Front::TypeStruct *typeStruct = (Front::TypeStruct*)memberNode->children[0]->type;
					for(unsigned int i=0; i<typeStruct->members.size(); i++) {
						if(typeStruct->members[i].name == memberNode->lexVal.s) {
							idx = i;
							break;
						}
					}

					procedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeStoreMem, b, a, typeStruct->members[idx].offset / 4));
				}

				// Return the resulting node
				result = b;
				break;

			case Node::NodeTypeCall:
				{
					// Emit code for each argument, building a list of resulting symbols
					std::vector<IR::Symbol*> args;
					for(unsigned int i=0; i<node->children[1]->children.size(); i++) {
						IR::Symbol *arg = processRValue(node->children[1]->children[i], context);
						args.push_back(arg);
					}

					// Emit argument store values for each argument
					for(unsigned int i=0; i<node->children[1]->children.size(); i++) {
						procedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeStoreArg, 0, args[i], i));
					}

					// Emit procedure call
					procedure->emit(new IR::EntryCall(context.program->findProcedure(node->children[0]->lexVal.s)));

					result = procedure->newTemp();
					if(procedure->returnsValue()) {
						// Assign return value to a new temporary
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadRet, result));
					}
					break;
				}

			case Node::NodeTypeArith:
				{
					result = procedure->newTemp();

					// Emit code for operator arguments
					std::vector<IR::Symbol*> arguments;
					for(unsigned int i=0; i<node->children.size(); i++) {
						arguments.push_back(processRValue(node->children[i], context));
					}

					// Emit the appropriate type of arithmetic operation
					switch(node->nodeSubtype) {
						case Node::NodeSubtypeAdd:
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeAdd, result, arguments[0], arguments[1]));
							break;

						case Node::NodeSubtypeSubtract:
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeSubtract, result, arguments[0], arguments[1]));
							break;

						case Node::NodeSubtypeMultiply:
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMult, result, arguments[0], arguments[1]));
							break;

						case Node::NodeSubtypeIncrement:
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, result, arguments[0]));
							procedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeAddImm, arguments[0], arguments[0], 1));
							break;

						case Node::NodeSubtypeDecrement:
							procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, result, arguments[0]));
							procedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeAddImm, arguments[0], arguments[0], -1));
							break;
					}
					break;
				}

			case Node::NodeTypeCompare:
				// Construct a new temporary to hold value
				result = procedure->newTemp();

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
					// Construct a new temporary to hold value
					result = procedure->newTemp();

					Node *arg = node->children[0];
					IR::Symbol *size = procedure->newTemp();
					if(arg->nodeType == Node::NodeTypeArray && arg->children.size() == 2) {
						// Array allocation: total size is typeSize * count
						Type *type = arg->children[0]->type;
						IR::Symbol *typeSize = procedure->newTemp();
						procedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeLoadImm, typeSize, 0, type->size));

						IR::Symbol *count = processRValue(arg->children[1], context);
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMult, size, typeSize, count));
					} else {
						// Single allocation: total size is typeSize
						Type *type = arg->type;
						procedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeLoadImm, size, 0, type->size));
					}

					// Emit new entry
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeNew, result, size));
					break;
				}

			case Node::NodeTypeArray:
				{
					result = procedure->newTemp();

					// Emit code to calculate the array's base address
					IR::Symbol *base = processRValue(node->children[0], context);

					// Emit code to calculate the array subscript
					IR::Symbol *subscript = processRValue(node->children[1], context);

					// Compute the offset into array memory based o subscript and type size
					IR::Symbol *offset = procedure->newTemp();
					IR::Symbol *size = procedure->newTemp();

					procedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeLoadImm, size, 0, node->type->size / 4));
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMult, offset, subscript, size));

					// Emit the load from the calculated memory location
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadMemInd, result, base, offset));
					break;
				}

			case Node::NodeTypeMember:
				{
					result = procedure->newTemp();

					IR::Symbol *base = processRValue(node->children[0], context);

					int idx;
					Front::TypeStruct *typeStruct = (Front::TypeStruct*)node->children[0]->type;
					for(unsigned int i=0; i<typeStruct->members.size(); i++) {
						if(typeStruct->members[i].name == node->lexVal.s) {
							idx = i;
							break;
						}
					}

					// Emit the load from the calculated memory location
					procedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeLoadMem, result, base, typeStruct->members[idx].offset / 4));
					break;
				}
		}

		return result;
	}
}
