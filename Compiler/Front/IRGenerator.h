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
	struct Node;

	class IRGenerator {
	public:
		IRGenerator();

		IR::Program *generate(Node *tree);

	private:
		void processNode(Node *node, IR::Program *program, IR::Procedure *procedure);
		IR::Symbol *processRValue(Node *node, IR::Program *program, IR::Procedure *procedure);
	};
}

#endif