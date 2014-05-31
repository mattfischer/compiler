#ifndef VM_PROGRAM_H
#define VM_PROGRAM_H

#include "VM/Instruction.h"

#include <vector>
#include <map>
#include <string>
#include <iostream>

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

		struct Relocation {
			enum Type {
				TypeAbsolute,
				TypeCall
			};

			int offset;
			Type type;
			std::string symbol;
		};
		std::vector<Relocation> relocations;

		void print(std::ostream &o);
	};
}

#endif