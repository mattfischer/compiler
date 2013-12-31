#ifndef BACK_CODE_GENERATOR_H
#define BACK_CODE_GENERATOR_H

#include <vector>
#include <map>

#include "VM/Program.h"

namespace IR {
	class Procedure;
	class Program;
}

namespace Back {
	class CodeGenerator {
	public:
		static VM::Program generate(IR::Program *irProgram);

	private:
		static void generateProcedure(IR::Procedure *procedure, std::vector<VM::Instruction> &instructions, const std::map<IR::Procedure*, int> &procedureMap);
	};
}
#endif