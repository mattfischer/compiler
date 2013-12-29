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
		IRGenerator();

		IR::Program *generate(SyntaxNode *tree);

	private:
		void processNode(SyntaxNode *node, IR::Procedure *procedure);
		IR::Symbol *processRValue(SyntaxNode *node, IR::Procedure *procedure);
	};
}

#endif