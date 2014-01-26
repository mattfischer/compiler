#include "Front/IRGenerator.h"

#include "Front/Node.h"
#include "Front/Type.h"

#include "IR/Program.h"
#include "IR/Procedure.h"
#include "IR/Entry.h"

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
			irProcedure->emit(new IR::EntryOneAddrImm(IR::Entry::TypePrologue, 0, 0));

			// Add local variables to the procedure
			const std::vector<Symbol*> locals = procedure->locals->symbols();
			for(unsigned int j=0; j<locals.size(); j++) {
				IR::Symbol *irSymbol = new IR::Symbol(locals[j]->name, locals[j]->type);
				irProcedure->addSymbol(irSymbol);

				for(unsigned int k=0; k<procedure->arguments.size(); k++) {
					if(procedure->arguments[k] == locals[j]) {
						// If this symbol is an argument, emit an argument load instruction
						irProcedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeLoadArg, irSymbol, 0, k));
					}
				}
			}

			// Emit procedure body
			processNode(procedure->body, irProgram, irProcedure);

			// Emit function epilogue
			irProcedure->entries().insert(irProcedure->entries().end(), new IR::EntryOneAddrImm(IR::Entry::TypeEpilogue, 0, 0));

			// Add procedure to procedure list
			irProgram->addProcedure(irProcedure);
		}

		return irProgram;
	}

	/*!
	 * \brief Process a node in the syntax tree
	 * \param node Node to process
	 * \param program Program being constructed
	 * \param procedure Procedure being constructed
	 */
	void IRGenerator::processNode(Node *node, IR::Program *program, IR::Procedure *procedure)
	{
		IR::Symbol *lhs, *rhs;

		switch(node->nodeType) {
			case Node::NodeTypeList:
				// Process each item in the list
				for(unsigned int i=0; i<node->children.size(); i++) {
					processNode(node->children[i], program, procedure);
				}
				break;

			case Node::NodeTypePrint:
				// Process RHS value
				rhs = processRValue(node->children[0], program, procedure);

				// Emit print instruction
				procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypePrint, 0, rhs));
				break;

			case Node::NodeTypeVarDecl:
				// No work to do, symbol was already added when creating the procedure
				break;

			case Node::NodeTypeIf:
				// Process statement predicate
				lhs = processRValue(node->children[0], program, procedure);
				if(node->children.size() == 2) {
					IR::EntryLabel *trueLabel = procedure->newLabel();
					IR::EntryLabel *nextLabel = procedure->newLabel();

					// Emit conditional jump instruction to either true label or label after statement
					procedure->emit(new IR::EntryCJump(lhs, trueLabel, nextLabel));

					// Process true body
					procedure->emit(trueLabel);
					processNode(node->children[1], program, procedure);

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
					processNode(node->children[1], program, procedure);
					procedure->emit(new IR::EntryJump(nextLabel));

					// Process false body
					procedure->emit(falseLabel);
					processNode(node->children[2], program, procedure);

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
					lhs = processRValue(node->children[0], program, procedure);
					procedure->emit(new IR::EntryCJump(lhs, mainLabel, nextLabel));

					// Emit body label
					procedure->emit(mainLabel);
					processNode(node->children[1], program, procedure);
					procedure->emit(new IR::EntryJump(testLabel));

					// Emit label following statement
					procedure->emit(nextLabel);
					break;
				}

			case Node::NodeTypeReturn:
				{
					// Emit code for return value
					rhs = processRValue(node->children[0], program, procedure);

					// Emit procedure return and jump to end block
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreRet, 0, rhs));
					procedure->emit(new IR::EntryJump(procedure->end()));

					// Emit label following statement
					IR::EntryLabel *label = procedure->newLabel();
					procedure->emit(label);
					break;
				}

			default:
				processRValue(node, program, procedure);
				break;
		}
	}

	/*!
	 * \brief Process an R-Value expression
	 * \param node Tree node to process
	 * \param program Program being constructed
	 * \param procedure Procedure being constructed
	 * \return Symbol containing value
	 */
	IR::Symbol *IRGenerator::processRValue(Node *node, IR::Program *program, IR::Procedure *procedure)
	{
		IR::Symbol *result;
		IR::Symbol *a, *b;

		switch(node->nodeType) {
			case Node::NodeTypeConstant:
				// Construct a temporary to contain the new value
				result = procedure->newTemp(node->type);
				procedure->emit(new IR::EntryOneAddrImm(IR::Entry::TypeLoadImm, result, node->lexVal.i));
				break;

			case Node::NodeTypeId:
				// Return the already-existing variable node
				result = procedure->findSymbol(node->lexVal.s);
				break;

			case Node::NodeTypeAssign:
				// If the LHS is a declaration, process it so that the symbol gets added
				if(node->children[0]->nodeType == Node::NodeTypeVarDecl) {
					processNode(node->children[0], program, procedure);
				}

				// Locate symbol to assign into
				a = procedure->findSymbol(node->children[0]->lexVal.s);

				// Emit code for R-Value of assignment
				b = processRValue(node->children[1], program, procedure);

				// Emit a move into the target symbol
				procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, a, b));

				// Return the resulting node
				result = b;
				break;

			case Node::NodeTypeCall:
				{
					// Emit code for each argument, building a list of resulting symbols
					std::vector<IR::Symbol*> args;
					for(unsigned int i=0; i<node->children[1]->children.size(); i++) {
						IR::Symbol *arg = processRValue(node->children[1]->children[i], program, procedure);
						args.push_back(arg);
					}

					// Emit argument store values for each argument
					for(unsigned int i=0; i<node->children[1]->children.size(); i++) {
						procedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeStoreArg, 0, args[i], i));
					}

					// Emit procedure call
					procedure->emit(new IR::EntryCall(program->findProcedure(node->children[0]->lexVal.s)));

					// Assign return value to a new temporary
					result = procedure->newTemp(node->type);
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadRet, result));
					break;
				}

			case Node::NodeTypeArith:
				// Construct a new temporary to hold value
				result = procedure->newTemp(node->type);

				// Emit code for operator arguments
				a = processRValue(node->children[0], program, procedure);
				b = processRValue(node->children[1], program, procedure);

				// Emit the appropriate type of arithmetic operation
				switch(node->nodeSubtype) {
					case Node::NodeSubtypeAdd:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeAdd, result, a, b));
						break;

					case Node::NodeSubtypeMultiply:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMult, result, a, b));
						break;
				}
				break;

			case Node::NodeTypeCompare:
				// Construct a new temporary to hold value
				result = procedure->newTemp(node->type);

				// Emit code for operator arguments
				a = processRValue(node->children[0], program, procedure);
				b = processRValue(node->children[1], program, procedure);

				// Emit the appropriate type of comparison operation
				switch(node->nodeSubtype) {
					case Node::NodeSubtypeEqual:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeEqual, result, a, b));
						break;

					case Node::NodeSubtypeNequal:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeNequal, result, a, b));
						break;
				}
				break;

			case Node::NodeTypeNew:
				// Construct a new temporary to hold value
				result = procedure->newTemp(node->type);

				Node *arg = node->children[0];
				IR::Symbol *size = procedure->newTemp(TypeInt);
				if(arg->nodeType == Node::NodeTypeArray && arg->children.size() == 2) {
					// Array allocation: total size is typeSize * count
					Type *type = arg->children[0]->type;
					IR::Symbol *typeSize = procedure->newTemp(TypeInt);
					procedure->emit(new IR::EntryOneAddrImm(IR::Entry::TypeLoadImm, typeSize, type->size));

					IR::Symbol *count = processRValue(arg->children[1], program, procedure);
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMult, size, typeSize, count));
				} else {
					// Single allocation: total size is typeSize
					Type *type = arg->type;
					procedure->emit(new IR::EntryOneAddrImm(IR::Entry::TypeLoadImm, size, type->size));
				}

				// Emit new entry
				procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeNew, result, size));
				break;
		}

		return result;
	}
}
