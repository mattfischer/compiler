#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include <string>

namespace IR {
	class Block;
	class Program;
	class Procedure;
	class Symbol;
}

namespace Front {
	struct SyntaxNode;

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
}

#endif