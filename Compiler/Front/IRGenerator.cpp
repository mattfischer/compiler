#include "Front/IRGenerator.h"

#include "Front/SyntaxNode.h"
#include "Front/Type.h"

#include "IR/Program.h"
#include "IR/Procedure.h"
#include "IR/Block.h"
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
		mProc->emit(new IR::EntryJump(mProc->end()->label));

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
				mProc->emit(new IR::EntryThreeAddr(IR::Entry::TypeLoad, lhs, rhs));
				break;

			case SyntaxNode::NodeTypeIf:
				lhs = processRValue(node->children[0]);
				if(node->numChildren == 2) {
					IR::Block *trueBlock = mProc->newBlock();
					IR::Block *nextBlock = mProc->newBlock();

					mProc->emit(new IR::EntryCJump(lhs, trueBlock->label, nextBlock->label));
					
					setCurrentBlock(trueBlock);
					processNode(node->children[1]);

					setCurrentBlock(trueBlock);
					mProc->emit(new IR::EntryJump(nextBlock->label));

					setCurrentBlock(nextBlock);
				} else {
					IR::Block *trueBlock = mProc->newBlock();
					IR::Block *falseBlock = mProc->newBlock();
					IR::Block *nextBlock = mProc->newBlock();

					mProc->emit(new IR::EntryCJump(lhs, trueBlock->label, falseBlock->label));

					setCurrentBlock(trueBlock);
					processNode(node->children[1]);
					mProc->emit(new IR::EntryJump(nextBlock->label));

					setCurrentBlock(falseBlock);
					processNode(node->children[2]);
					mProc->emit(new IR::EntryJump(nextBlock->label));

					setCurrentBlock(nextBlock);
				}
				break;

			case SyntaxNode::NodeTypeWhile:
				{
					IR::Block *testBlock = mProc->newBlock();
					IR::Block *mainBlock = mProc->newBlock();
					IR::Block *nextBlock = mProc->newBlock();

					mProc->emit(new IR::EntryJump(testBlock->label));
					setCurrentBlock(testBlock);
					lhs = processRValue(node->children[0]);
					mProc->emit(new IR::EntryCJump(lhs, mainBlock->label, nextBlock->label));

					setCurrentBlock(mainBlock);
					processNode(node->children[1]);
					mProc->emit(new IR::EntryJump(testBlock->label));

					setCurrentBlock(nextBlock);
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
				mProc->emit(new IR::EntryImm(result, node->lexVal._int));
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

	void IRGenerator::setCurrentBlock(IR::Block *block)
	{
		mProc->setCurrentBlock(block);
	}
}
