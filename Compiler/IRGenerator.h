#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "IR.h"
#include "SyntaxNode.h"
#include "Type.h"

#include <string>

class IRGenerator {
public:
	IRGenerator(SyntaxNode *tree);

	IR *generate();

private:
	SyntaxNode *mTree;
	IR *mIR;
	IR::Procedure *mCurrentProcedure;

	void processNode(SyntaxNode *node);
	void emit(IR::Entry::Type type, void *lhs, void *rhs1 = 0, void *rhs2 = 0);
	IR::Symbol *processRValue(SyntaxNode *node);
	void setCurrentBlock(IR::Block *block);
};

#endif