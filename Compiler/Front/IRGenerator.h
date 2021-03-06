#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "IR/Program.h"
#include "IR/Procedure.h"
#include "IR/Symbol.h"

#include "Front/Node.h"
#include "Front/Program.h"

#include <string>

namespace Front {
	/*!
	 * \brief Generates an IR instruction stream from a program syntax tree
	 */
	class IRGenerator {
	public:
		std::unique_ptr<IR::Program> generate(const Program &program);

	private:
		struct Context {
			IR::Procedure &procedure;
			IR::EntryLabel *breakTarget;
			IR::EntryLabel *continueTarget;
			IR::Symbol *object;
		};

		void processNode(Node &node, Context &context);
		IR::Symbol *processRValue(Node &node, Context &context);
	};
}

#endif