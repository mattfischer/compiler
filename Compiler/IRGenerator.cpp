#include "IRGenerator.h"

IRGenerator::IRGenerator(SyntaxNode *tree)
{
	mTree = tree;
	mIR = new IR;
	mCurrentProcedure = mIR->main();
}

IR *IRGenerator::generate()
{
	processNode(mTree);
	emit(IR::Entry::TypeJump, mCurrentProcedure->end());

	mCurrentProcedure->topoSort();

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
			emit(IR::Entry::TypePrint, lhs);
			break;

		case SyntaxNode::NodeTypeVarDecl:
			mCurrentProcedure->addSymbol(node->children[1]->lexVal._id, Type::find(node->children[0]->lexVal._id));
			break;

		case SyntaxNode::NodeTypeAssign:
			lhs = mCurrentProcedure->findSymbol(node->children[0]->lexVal._id);
			rhs = processRValue(node->children[1]);
			emit(IR::Entry::TypeLoad, lhs, rhs);
			break;

		case SyntaxNode::NodeTypeIf:
			lhs = processRValue(node->children[0]);
			if(node->numChildren == 2) {
				IR::Block *trueBlock = mCurrentProcedure->newBlock();
				IR::Block *nextBlock = mCurrentProcedure->newBlock();

				emit(IR::Entry::TypeCJump, lhs, trueBlock, nextBlock);
				
				setCurrentBlock(trueBlock);
				processNode(node->children[1]);

				setCurrentBlock(trueBlock);
				emit(IR::Entry::TypeJump, nextBlock);

				setCurrentBlock(nextBlock);
			} else {
				IR::Block *trueBlock = mCurrentProcedure->newBlock();
				IR::Block *falseBlock = mCurrentProcedure->newBlock();
				IR::Block *nextBlock = mCurrentProcedure->newBlock();

				emit(IR::Entry::TypeCJump, lhs, trueBlock, falseBlock);

				setCurrentBlock(trueBlock);
				processNode(node->children[1]);
				emit(IR::Entry::TypeJump, nextBlock);

				setCurrentBlock(falseBlock);
				processNode(node->children[2]);
				emit(IR::Entry::TypeJump, nextBlock);

				setCurrentBlock(nextBlock);
			}
			break;

		case SyntaxNode::NodeTypeWhile:
			{
				IR::Block *testBlock = mCurrentProcedure->newBlock();
				IR::Block *mainBlock = mCurrentProcedure->newBlock();
				IR::Block *nextBlock = mCurrentProcedure->newBlock();

				emit(IR::Entry::TypeJump, testBlock);
				setCurrentBlock(testBlock);
				lhs = processRValue(node->children[0]);
				emit(IR::Entry::TypeCJump, lhs, mainBlock, nextBlock);

				setCurrentBlock(mainBlock);
				processNode(node->children[1]);
				emit(IR::Entry::TypeJump, testBlock);

				setCurrentBlock(nextBlock);
				break;
			}
	}
}

void IRGenerator::emit(IR::Entry::Type type, void *lhs, void *rhs1, void *rhs2)
{
	mCurrentProcedure->emit(type, lhs, rhs1, rhs2);
}

IR::Symbol *IRGenerator::processRValue(SyntaxNode *node)
{
	IR::Symbol *result;
	IR::Symbol *a, *b;

	switch(node->nodeType) {
		case SyntaxNode::NodeTypeConstant:
			result = mCurrentProcedure->newTemp(node->type);
			emit(IR::Entry::TypeLoadImm, result, (void*)node->lexVal._int);
			break;

		case SyntaxNode::NodeTypeId:
			result = mCurrentProcedure->findSymbol(node->lexVal._id);
			break;

		case SyntaxNode::NodeTypeArith:
			result = mCurrentProcedure->newTemp(node->type);
			a = processRValue(node->children[0]);
			b = processRValue(node->children[1]);
			switch(node->nodeSubtype) {
				case SyntaxNode::NodeSubtypeAdd:
					emit(IR::Entry::TypeAdd, result, a, b);
					break;

				case SyntaxNode::NodeSubtypeMultiply:
					emit(IR::Entry::TypeMult, result, a, b);
					break;
			}
			break;

		case SyntaxNode::NodeTypeCompare:
			result = mCurrentProcedure->newTemp(node->type);
			a = processRValue(node->children[0]);
			b = processRValue(node->children[1]);
			switch(node->nodeSubtype) {
				case SyntaxNode::NodeSubtypeEqual:
					emit(IR::Entry::TypeEqual, result, a, b);
					break;

				case SyntaxNode::NodeSubtypeNequal:
					emit(IR::Entry::TypeNequal, result, a, b);
					break;
			}
			break;
	}

	return result;
}

void IRGenerator::setCurrentBlock(IR::Block *block)
{
	mCurrentProcedure->setCurrentBlock(block);
}