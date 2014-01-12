#ifndef BACK_CODE_GENERATOR_H
#define BACK_CODE_GENERATOR_H

#include "VM/Program.h"

#include "IR/Procedure.h"
#include "IR/Program.h"

#include <vector>
#include <map>

namespace Back {
	class CodeGenerator {
	public:
		static VM::Program generate(IR::Program *irProgram);

	private:
		static void generateProcedure(IR::Procedure *procedure, std::vector<VM::Instruction> &instructions, const std::map<IR::Procedure*, int> &procedureMap);
	};
}
#endif