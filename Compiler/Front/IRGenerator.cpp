#include "Front/IRGenerator.h"

#include "Front/SyntaxNode.h"
#include "Front/Type.h"

#include "IR/Program.h"
#include "IR/Procedure.h"
#include "IR/Entry.h"

namespace Front {
	IRGenerator::IRGenerator(SyntaxNode *tree)
	{
		mTree = tree;
		mIR = new IR::Program;
		mProc = mIR->main();
	}

	IR::Program *IRGenerator::generate()
	{
		processNode(mTree);

		return mIR;
	}

	void IRGenerator::processNode(SyntaxNode *node)
	{
		IR::Symbol *lhs, *rhs;

		switch(node->nodeType) {
			case SyntaxNode::NodeTypeStatementList:
				for(int i=0; i<node->numChildren; i++) {
					processNode(node->children[i]);
				}
				break;

			case SyntaxNode::NodeTypeProcedureList:
				for(int i=0; i<node->numChildren; i++) {
					SyntaxNode *proc = node->children[i];
					IR::Procedure *procedure = new IR::Procedure(proc->children[1]->lexVal._id);
					mProc = procedure;
					processNode(proc->children[2]);
					mIR->addProcedure(procedure);
				}
				break;

			case SyntaxNode::NodeTypePrintStatement:
				rhs = processRValue(node->children[0]);
				mProc->emit(new IR::EntryThreeAddr(IR::Entry::TypePrint, 0, rhs));
				break;

			case SyntaxNode::NodeTypeVarDecl:
				mProc->addSymbol(node->children[1]->lexVal._id, Type::find(node->children[0]->lexVal._id));
				break;

			case SyntaxNode::NodeTypeAssign:
				lhs = mProc->findSymbol(node->children[0]->lexVal._id);
				rhs = processRValue(node->children[1]);
				if(rhs != lhs) {
					mProc->emit(new IR::EntryThreeAddr(IR::Entry::TypeMove, lhs, rhs));
				}
				break;

			case SyntaxNode::NodeTypeIf:
				lhs = processRValue(node->children[0]);
				if(node->numChildren == 2) {
					IR::EntryLabel *trueLabel = mProc->newLabel();
					IR::EntryLabel *nextLabel = mProc->newLabel();

					mProc->emit(new IR::EntryCJump(lhs, trueLabel, nextLabel));
					mProc->emit(trueLabel);
					processNode(node->children[1]);
					mProc->emit(nextLabel);
				} else {
					IR::EntryLabel *trueLabel = mProc->newLabel();
					IR::EntryLabel *falseLabel = mProc->newLabel();
					IR::EntryLabel *nextLabel = mProc->newLabel();

					mProc->emit(new IR::EntryCJump(lhs, trueLabel, falseLabel));
					mProc->emit(trueLabel);
					processNode(node->children[1]);
					mProc->emit(new IR::EntryJump(nextLabel));
					mProc->emit(falseLabel);
					processNode(node->children[2]);
					mProc->emit(nextLabel);
				}
				break;

			case SyntaxNode::NodeTypeWhile:
				{
					IR::EntryLabel *testLabel = mProc->newLabel();
					IR::EntryLabel *mainLabel = mProc->newLabel();
					IR::EntryLabel *nextLabel = mProc->newLabel();

					mProc->emit(testLabel);
					lhs = processRValue(node->children[0]);
					mProc->emit(new IR::EntryCJump(lhs, mainLabel, nextLabel));
					mProc->emit(mainLabel);

					processNode(node->children[1]);
					mProc->emit(new IR::EntryJump(testLabel));
					mProc->emit(nextLabel);
					break;
				}
		}
	}

	IR::Symbol *IRGenerator::processRValue(SyntaxNode *node)
	{
		IR::Symbol *result;
		IR::Symbol *a, *b;

		switch(node->nodeType) {
			case SyntaxNode::NodeTypeConstant:
				result = mProc->newTemp(node->type);
				mProc->emit(new IR::EntryOneAddrImm(IR::Entry::TypeLoadImm, result, node->lexVal._int));
				break;

			case SyntaxNode::NodeTypeId:
				result = mProc->findSymbol(node->lexVal._id);
				break;

			case SyntaxNode::NodeTypeArith:
				result = mProc->newTemp(node->type);
				a = processRValue(node->children[0]);
				b = processRValue(node->children[1]);
				switch(node->nodeSubtype) {
					case SyntaxNode::NodeSubtypeAdd:
						mProc->emit(new IR::EntryThreeAddr(IR::Entry::TypeAdd, result, a, b));
						break;

					case SyntaxNode::NodeSubtypeMultiply:
						mProc->emit(new IR::EntryThreeAddr(IR::Entry::TypeMult, result, a, b));
						break;
				}
				break;

			case SyntaxNode::NodeTypeCompare:
				result = mProc->newTemp(node->type);
				a = processRValue(node->children[0]);
				b = processRValue(node->children[1]);
				switch(node->nodeSubtype) {
					case SyntaxNode::NodeSubtypeEqual:
						mProc->emit(new IR::EntryThreeAddr(IR::Entry::TypeEqual, result, a, b));
						break;

					case SyntaxNode::NodeSubtypeNequal:
						mProc->emit(new IR::EntryThreeAddr(IR::Entry::TypeNequal, result, a, b));
						break;
				}
				break;
		}

		return result;
	}
}
