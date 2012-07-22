#ifndef VM_INTERP_H
#define VM_INTERP_H

#include "VM/Instruction.h"

#include <vector>

namespace VM {
	class Interp {
	public:
		static void run(std::vector<VM::Instruction> &instructions);
	};
}
#endif