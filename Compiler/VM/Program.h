#ifndef VM_PROGRAM_H
#define VM_PROGRAM_H

#include "VM/Instruction.h"

#include <vector>
#include <map>
#include <string>

/*!
 * \brief A virtual machine that is targeted by the compiler
 */
namespace VM {
	/*!
	 * \brief A program to be executed by the VM
	 */
	struct Program {
		std::vector<unsigned char> instructions; //!< Instruction list
		std::map<std::string, int> symbols;
		std::map<int, std::string> imports;

		void print();
	};
}

#endif