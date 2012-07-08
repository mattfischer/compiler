#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "SyntaxNode.h"
#include "Type.h"

#include <string>

namespace IR {
	struct Block;
	struct Program;
	class Procedure;
	struct Symbol;
}

class IRGenerator {
public:
	IRGenerator(SyntaxNode *tree);

	IR::Program *generate();

private:
	SyntaxNode *mTree;
	IR::Program *mIR;
	IR::Procedure *mProc;

	void processNode(SyntaxNode *node);
	IR::Symbol *processRValue(SyntaxNode *node);
	void setCurrentBlock(IR::Block *block);
};

#endif