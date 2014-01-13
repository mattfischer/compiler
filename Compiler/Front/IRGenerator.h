#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "IR/Program.h"
#include "IR/Procedure.h"
#include "IR/Symbol.h"

#include "Front/Node.h"

#include <string>

namespace Front {
	/*!
	 * \brief Generates an IR instruction stream from a program syntax tree
	 */
	class IRGenerator {
	public:
		IR::Program *generate(Node *tree);

	private:
		void processNode(Node *node, IR::Program *program, IR::Procedure *procedure);
		IR::Symbol *processRValue(Node *node, IR::Program *program, IR::Procedure *procedure);
	};
}

#endif