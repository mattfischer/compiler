#ifndef VM_PROGRAM_H
#define VM_PROGRAM_H

#include "VM/Instruction.h"

#include <vector>

/*!
 * \brief A virtual machine that is targeted by the compiler
 */
namespace VM {
	/*!
	 * \brief A program to be executed by the VM
	 */
	struct Program {
		std::vector<unsigned char> instructions; //!< Instruction list
		int start; //!< Program start point
	};
}

#endif