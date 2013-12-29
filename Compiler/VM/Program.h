#ifndef VM_PROGRAM_H
#define VM_PROGRAM_H

#include "VM/Instruction.h"

#include <vector>

namespace VM {
	struct Program {
		std::vector<Instruction> instructions;
		int start;
	};
}

#endif