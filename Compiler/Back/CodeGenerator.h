#ifndef BACK_CODE_GENERATOR_H
#define BACK_CODE_GENERATOR_H

#include <vector>

#include "VM/Instruction.h"

namespace IR {
	class Procedure;
	class Program;
}

namespace Back {
	class CodeGenerator {
	public:
		static std::vector<VM::Instruction> generate(IR::Program *program);

	private:
		static void generateProcedure(IR::Procedure *procedure, std::vector<VM::Instruction> &instructions);
	};
}
#endif