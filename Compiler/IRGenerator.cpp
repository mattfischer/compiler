#include "IRGenerator.h"

IRGenerator::IRGenerator(SyntaxNode *tree)
{
	mTree = tree;
	mIR = new IR;
	mProc = mIR->main();
}

IR *IRGenerator::generate()
{
	processNode(mTree);
	mProc->emitJump(mProc->end());

	mProc->topoSort();

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
			lhs = processRValue(node->children[0]);
			mProc->emitThreeAddr(IR::Entry::TypePrint, lhs);
			break;

		case SyntaxNode::NodeTypeVarDecl:
			mProc->addSymbol(node->children[1]->lexVal._id, Type::find(node->children[0]->lexVal._id));
			break;

		case SyntaxNode::NodeTypeAssign:
			lhs = mProc->findSymbol(node->children[0]->lexVal._id);
			rhs = processRValue(node->children[1]);
			mProc->emitThreeAddr(IR::Entry::TypeLoad, lhs, rhs);
			break;

		case SyntaxNode::NodeTypeIf:
			lhs = processRValue(node->children[0]);
			if(node->numChildren == 2) {
				IR::Block *trueBlock = mProc->newBlock();
				IR::Block *nextBlock = mProc->newBlock();

				mProc->emitCJump(lhs, trueBlock, nextBlock);
				
				setCurrentBlock(trueBlock);
				processNode(node->children[1]);

				setCurrentBlock(trueBlock);
				mProc->emitJump(nextBlock);

				setCurrentBlock(nextBlock);
			} else {
				IR::Block *trueBlock = mProc->newBlock();
				IR::Block *falseBlock = mProc->newBlock();
				IR::Block *nextBlock = mProc->newBlock();

				mProc->emitCJump(lhs, trueBlock, falseBlock);

				setCurrentBlock(trueBlock);
				processNode(node->children[1]);
				mProc->emitJump(nextBlock);

				setCurrentBlock(falseBlock);
				processNode(node->children[2]);
				mProc->emitJump(nextBlock);

				setCurrentBlock(nextBlock);
			}
			break;

		case SyntaxNode::NodeTypeWhile:
			{
				IR::Block *testBlock = mProc->newBlock();
				IR::Block *mainBlock = mProc->newBlock();
				IR::Block *nextBlock = mProc->newBlock();

				mProc->emitJump(testBlock);
				setCurrentBlock(testBlock);
				lhs = processRValue(node->children[0]);
				mProc->emitCJump(lhs, mainBlock, nextBlock);

				setCurrentBlock(mainBlock);
				processNode(node->children[1]);
				mProc->emitJump(testBlock);

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
			mProc->emitImm(IR::Entry::TypeLoadImm, result, node->lexVal._int);
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
					mProc->emitThreeAddr(IR::Entry::TypeAdd, result, a, b);
					break;

				case SyntaxNode::NodeSubtypeMultiply:
					mProc->emitThreeAddr(IR::Entry::TypeMult, result, a, b);
					break;
			}
			break;

		case SyntaxNode::NodeTypeCompare:
			result = mProc->newTemp(node->type);
			a = processRValue(node->children[0]);
			b = processRValue(node->children[1]);
			switch(node->nodeSubtype) {
				case SyntaxNode::NodeSubtypeEqual:
					mProc->emitThreeAddr(IR::Entry::TypeEqual, result, a, b);
					break;

				case SyntaxNode::NodeSubtypeNequal:
					mProc->emitThreeAddr(IR::Entry::TypeNequal, result, a, b);
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