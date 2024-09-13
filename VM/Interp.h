#ifndef VM_INTERP_H
#define VM_INTERP_H

#include "VM/Program.h"

#include <vector>
#include <iostream>

namespace VM {
	/*!
	 * \brief Interpreter for VM program
	 */
	class Interp {
	public:
		static void run(const VM::Program &program, std::ostream &o);
	};
}
#endif