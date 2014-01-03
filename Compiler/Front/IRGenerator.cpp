#include "Front/IRGenerator.h"

#include "Front/SyntaxNode.h"
#include "Front/Type.h"

#include "IR/Program.h"
#include "IR/Procedure.h"
#include "IR/Entry.h"

namespace Front {
	IRGenerator::IRGenerator()
	{
	}

	IR::Program *IRGenerator::generate(SyntaxNode *tree)
	{
		IR::Program *program = new IR::Program;

		for(int i=0; i<tree->numChildren; i++) {
			SyntaxNode *proc = tree->children[i];
			IR::Procedure *procedure = new IR::Procedure(proc->lexVal._id);
			SyntaxNode *args = proc->children[1];
			for(int j=0; j<args->numChildren; j++) {
				IR::Symbol *symbol = procedure->addSymbol(args->children[j]->lexVal._id, Front::Type::find(args->children[j]->children[0]->lexVal._id));
				procedure->emit(new IR::EntryOneAddrImm(IR::Entry::TypeArgument, symbol, j));
			}
			processNode(proc->children[2], program, procedure);
			program->addProcedure(procedure);
		}

		return program;
	}

	void IRGenerator::processNode(SyntaxNode *node, IR::Program *program, IR::Procedure *procedure)
	{
		IR::Symbol *lhs, *rhs;

		switch(node->nodeType) {
			case SyntaxNode::NodeTypeList:
				for(int i=0; i<node->numChildren; i++) {
					processNode(node->children[i], program, procedure);
				}
				break;

			case SyntaxNode::NodeTypePrint:
				rhs = processRValue(node->children[0], program, procedure);
				procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypePrint, 0, rhs));
				break;

			case SyntaxNode::NodeTypeCall:
				{
					std::vector<IR::Symbol*> args;
					for(int i=0; i<node->children[1]->numChildren; i++) {
						IR::Symbol *arg = processRValue(node->children[1]->children[i], program, procedure);
						args.push_back(arg);
					}

					IR::Symbol **argList = args.size() > 0 ? &args[0] : 0;
					procedure->emit(new IR::EntryCall(0, program->findProcedure(node->children[0]->lexVal._id), argList, (int)args.size()));
					break;
				}
			case SyntaxNode::NodeTypeVarDecl:
				procedure->addSymbol(node->lexVal._id, Type::find(node->children[0]->lexVal._id));
				break;

			case SyntaxNode::NodeTypeAssign:
				lhs = procedure->findSymbol(node->children[0]->lexVal._id);
				rhs = processRValue(node->children[1], program, procedure);
				if(rhs != lhs) {
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, lhs, rhs));
				}
				break;

			case SyntaxNode::NodeTypeIf:
				lhs = processRValue(node->children[0], program, procedure);
				if(node->numChildren == 2) {
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

			case SyntaxNode::NodeTypeWhile:
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

			case SyntaxNode::NodeTypeReturn:
				{
					rhs = processRValue(node->children[0], program, procedure);
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeReturn, 0, rhs));
					break;
				}
		}
	}

	IR::Symbol *IRGenerator::processRValue(SyntaxNode *node, IR::Program *program, IR::Procedure *procedure)
	{
		IR::Symbol *result;
		IR::Symbol *a, *b;

		switch(node->nodeType) {
			case SyntaxNode::NodeTypeConstant:
				result = procedure->newTemp(node->type);
				procedure->emit(new IR::EntryOneAddrImm(IR::Entry::TypeLoadImm, result, node->lexVal._int));
				break;

			case SyntaxNode::NodeTypeId:
				result = procedure->findSymbol(node->lexVal._id);
				break;

			case SyntaxNode::NodeTypeCall:
				{
					std::vector<IR::Symbol*> args;
					for(int i=0; i<node->children[1]->numChildren; i++) {
						IR::Symbol *arg = processRValue(node->children[1]->children[i], program, procedure);
						args.push_back(arg);
					}
					IR::Symbol **argList = args.size() > 0 ? &args[0] : 0;

					result = procedure->newTemp(node->type);
					procedure->emit(new IR::EntryCall(result, program->findProcedure(node->children[0]->lexVal._id), argList, (int)args.size()));
					break;
				}

			case SyntaxNode::NodeTypeArith:
				result = procedure->newTemp(node->type);
				a = processRValue(node->children[0], program, procedure);
				b = processRValue(node->children[1], program, procedure);
				switch(node->nodeSubtype) {
					case SyntaxNode::NodeSubtypeAdd:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeAdd, result, a, b));
						break;

					case SyntaxNode::NodeSubtypeMultiply:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMult, result, a, b));
						break;
				}
				break;

			case SyntaxNode::NodeTypeCompare:
				result = procedure->newTemp(node->type);
				a = processRValue(node->children[0], program, procedure);
				b = processRValue(node->children[1], program, procedure);
				switch(node->nodeSubtype) {
					case SyntaxNode::NodeSubtypeEqual:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeEqual, result, a, b));
						break;

					case SyntaxNode::NodeSubtypeNequal:
						procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeNequal, result, a, b));
						break;
				}
				break;
		}

		return result;
	}
}
