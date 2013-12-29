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
			IR::Procedure *procedure = new IR::Procedure(proc->children[1]->lexVal._id);
			processNode(proc->children[2], procedure);
			program->addProcedure(procedure);
		}

		return program;
	}

	void IRGenerator::processNode(SyntaxNode *node, IR::Procedure *procedure)
	{
		IR::Symbol *lhs, *rhs;

		switch(node->nodeType) {
			case SyntaxNode::NodeTypeStatementList:
				for(int i=0; i<node->numChildren; i++) {
					processNode(node->children[i], procedure);
				}
				break;

			case SyntaxNode::NodeTypePrintStatement:
				rhs = processRValue(node->children[0], procedure);
				procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypePrint, 0, rhs));
				break;

			case SyntaxNode::NodeTypeVarDecl:
				procedure->addSymbol(node->children[1]->lexVal._id, Type::find(node->children[0]->lexVal._id));
				break;

			case SyntaxNode::NodeTypeAssign:
				lhs = procedure->findSymbol(node->children[0]->lexVal._id);
				rhs = processRValue(node->children[1], procedure);
				if(rhs != lhs) {
					procedure->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, lhs, rhs));
				}
				break;

			case SyntaxNode::NodeTypeIf:
				lhs = processRValue(node->children[0], procedure);
				if(node->numChildren == 2) {
					IR::EntryLabel *trueLabel = procedure->newLabel();
					IR::EntryLabel *nextLabel = procedure->newLabel();

					procedure->emit(new IR::EntryCJump(lhs, trueLabel, nextLabel));
					procedure->emit(trueLabel);
					processNode(node->children[1], procedure);
					procedure->emit(nextLabel);
				} else {
					IR::EntryLabel *trueLabel = procedure->newLabel();
					IR::EntryLabel *falseLabel = procedure->newLabel();
					IR::EntryLabel *nextLabel = procedure->newLabel();

					procedure->emit(new IR::EntryCJump(lhs, trueLabel, falseLabel));
					procedure->emit(trueLabel);
					processNode(node->children[1], procedure);
					procedure->emit(new IR::EntryJump(nextLabel));
					procedure->emit(falseLabel);
					processNode(node->children[2], procedure);
					procedure->emit(nextLabel);
				}
				break;

			case SyntaxNode::NodeTypeWhile:
				{
					IR::EntryLabel *testLabel = procedure->newLabel();
					IR::EntryLabel *mainLabel = procedure->newLabel();
					IR::EntryLabel *nextLabel = procedure->newLabel();

					procedure->emit(testLabel);
					lhs = processRValue(node->children[0], procedure);
					procedure->emit(new IR::EntryCJump(lhs, mainLabel, nextLabel));
					procedure->emit(mainLabel);

					processNode(node->children[1], procedure);
					procedure->emit(new IR::EntryJump(testLabel));
					procedure->emit(nextLabel);
					break;
				}
		}
	}

	IR::Symbol *IRGenerator::processRValue(SyntaxNode *node, IR::Procedure *procedure)
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

			case SyntaxNode::NodeTypeArith:
				result = procedure->newTemp(node->type);
				a = processRValue(node->children[0], procedure);
				b = processRValue(node->children[1], procedure);
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
				a = processRValue(node->children[0], procedure);
				b = processRValue(node->children[1], procedure);
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
