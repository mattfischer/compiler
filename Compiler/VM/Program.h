#ifndef VM_PROGRAM_H
#define VM_PROGRAM_H

#include "VM/Instruction.h"
#include "VM/OrcFile.h"

#include <vector>
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

		struct Relocation {
			enum Type {
				TypeAbsolute,
				TypeCall,
				TypeAddPCRel
			};

			int offset;
			Type type;
			std::string symbol;
		};
		std::vector<Relocation> relocations;

		Program();
		Program(const OrcFile &file);
		Program(const std::string &filename);

		void read(const OrcFile &file);
		void write(OrcFile &file);
		void write(const std::string &filename);

		void print(std::ostream &o);
	};
}

#endif