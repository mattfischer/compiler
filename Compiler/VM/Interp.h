#ifndef VM_INTERP_H
#define VM_INTERP_H

#include "VM/Program.h"

#include <vector>

namespace VM {
	class Interp {
	public:
		static void run(const VM::Program &program);
	};
}
#endif