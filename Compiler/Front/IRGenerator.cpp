#include "Front/IRGenerator.h"

#include "Front/Node.h"
#include "Front/Type.h"

#include "IR/Program.h"
#include "IR/Procedure.h"
#include "IR/Entry.h"

namespace Front {
	IRGenerator::IRGenerator()
	{
	}

	IR::Program *IRGenerator::generate(Node *tree)
	{
		IR::Program *program = new IR::Program;

		for(unsigned int i=0; i<tree->children.size(); i++) {
			Node *proc = tree->children[i];
			IR::Procedure *procedure = new IR::Procedure(proc->lexVal.s);
			procedure->emit(new IR::EntryOneAddrImm(IR::Entry::TypePrologue, 0, 0));
			Node *args = proc->children[1];
			for(unsigned int j=0; j<args->children.size(); j++) {
				IR::Symbol *symbol = procedure->addSymbol(args->children[j]->lexVal.s, Front::Type::find(args->children[j]->children[0]->lexVal.s));
				procedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeLoadArg, symbol, 0, j));
			}
			processNode(proc->children[2], program, procedure);
			procedure->entries().insert(procedure->entries().end(), new IR::EntryOneAddrImm(IR::Entry::TypeEpilogue, 0, 0));
			program->addProcedure(procedure);
		}

		return program;
	}

	void IRGenerator::processNode(Node *node, IR::Program *program, IR::Procedure *procedure)
	{
		IR::Symbol *lhs, *rhs;

		switch(node->nodeType) {
			case Node::NodeTypeList:
				for(unsigned int i=0; i<node->children.size(); i++) {
					processNode(node->children[i], program, procedure);
				}
				break;

			case Node::NodeTypePrint:
				rhs = processRValue(node->children[0], program, procedure);
				procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypePrint, 0, rhs));
				break;

			case Node::NodeTypeVarDecl:
				procedure->addSymbol(node->lexVal.s, Type::find(node->children[0]->lexVal.s));
				break;

			case Node::NodeTypeIf:
				lhs = processRValue(node->children[0], program, procedure);
				if(node->children.size() == 2) {
					IR::EntryLabel *trueLabel = procedure->newLabel();
					IR::EntryLabel *nextLabel = procedure->newLabel();

					procedure->emit(new IR::EntryCJump(lhs, trueLabel, nextLabel));
					procedure->emit(trueLabel);
					processNode(node->children[1], program, procedure);
					procedure->emit(nextLabel);
				} else {
					IR::EntryLabel *trueLabel = procedure->newLabel();
					IR::EntryLabel *falseLabel = procedure->newLabel();
					IR::EntryLabel *nextLabel = procedure->newLabel();

					procedure->emit(new IR::EntryCJump(lhs, trueLabel, falseLabel));
					procedure->emit(trueLabel);
					processNode(node->children[1], program, procedure);
					procedure->emit(new IR::EntryJump(nextLabel));
					procedure->emit(falseLabel);
					processNode(node->children[2], program, procedure);
					procedure->emit(nextLabel);
				}
				break;

			case Node::NodeTypeWhile:
				{
					IR::EntryLabel *testLabel = procedure->newLabel();
					IR::EntryLabel *mainLabel = procedure->newLabel();
					IR::EntryLabel *nextLabel = procedure->newLabel();

					procedure->emit(testLabel);
					lhs = processRValue(node->children[0], program, procedure);
					procedure->emit(new IR::EntryCJump(lhs, mainLabel, nextLabel));
					procedure->emit(mainLabel);

					processNode(node->children[1], program, procedure);
					procedure->emit(new IR::EntryJump(testLabel));
					procedure->emit(nextLabel);
					break;
				}

			case Node::NodeTypeReturn:
				{
					rhs = processRValue(node->children[0], program, procedure);
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeStoreRet, 0, rhs));
					procedure->emit(new IR::EntryJump(procedure->end()));

					IR::EntryLabel *label = procedure->newLabel();
					procedure->emit(label);
					break;
				}

			default:
				processRValue(node, program, procedure);
				break;
		}
	}

	IR::Symbol *IRGenerator::processRValue(Node *node, IR::Program *program, IR::Procedure *procedure)
	{
		IR::Symbol *result;
		IR::Symbol *a, *b;

		switch(node->nodeType) {
			case Node::NodeTypeConstant:
				result = procedure->newTemp(node->type);
				procedure->emit(new IR::EntryOneAddrImm(IR::Entry::TypeLoadImm, result, node->lexVal.i));
				break;

			case Node::NodeTypeId:
				result = procedure->findSymbol(node->lexVal.s);
				break;

			case Node::NodeTypeAssign:
				a = procedure->findSymbol(node->children[0]->lexVal.s);
				b = processRValue(node->children[1], program, procedure);
				if(a != b) {
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, a, b));
				}
				result = b;
				break;

			case Node::NodeTypeCall:
				{
					std::vector<IR::Symbol*> args;
					for(unsigned int i=0; i<node->children[1]->children.size(); i++) {
						IR::Symbol *arg = processRValue(node->children[1]->children[i], program, procedure);
						args.push_back(arg);
					}

					for(unsigned int i=0; i<node->children[1]->children.size(); i++) {
						procedure->emit(new IR::EntryTwoAddrImm(IR::Entry::TypeStoreArg, 0, args[i], i));
					}

					procedure->emit(new IR::EntryCall(program->findProcedure(node->children[0]->lexVal.s)));
					result = procedure->newTemp(node->type);
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoadRet, result));
					break;
				}

			case Node::NodeTypeArith:
				result = procedure->newTemp(node->type);
				a = processRValue(node->children[0], program, procedure);
				b = processRValue(node->children[1], program, procedure);
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
				result = procedure->newTemp(node->type);
				a = processRValue(node->children[0], program, procedure);
				b = processRValue(node->children[1], program, procedure);
				switch(node->nodeSubtype) {
					case Node::NodeSubtypeEqual:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeEqual, result, a, b));
						break;

					case Node::NodeSubtypeNequal:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeNequal, result, a, b));
						break;
				}
				break;
		}

		return result;
	}
}
