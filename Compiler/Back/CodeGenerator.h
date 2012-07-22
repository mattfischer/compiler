#ifndef BACK_CODE_GENERATOR_H
#define BACK_CODE_GENERATOR_H

#include <vector>

#include "VM/Instruction.h"

namespace IR {
	class Procedure;
}

namespace Back {
	class CodeGenerator {
	public:
		static std::vector<VM::Instruction> generate(IR::Procedure *procedure);
	};
}
#endif