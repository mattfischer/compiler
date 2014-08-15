#ifndef VM_PROGRAM_H
#define VM_PROGRAM_H

#include "VM/Instruction.h"
#include "VM/OrcFile.h"

#include "Front/ExportInfo.h"

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
			enum class Type {
				Absolute,
				Call,
				AddPCRel
			};

			int offset;
			Type type;
			std::string symbol;
		};
		std::vector<Relocation> relocations;

		Front::ExportInfo *exportInfo;

		Program();
		Program(const OrcFile &file);
		Program(const std::string &filename);

		void read(const OrcFile &file);
		void write(OrcFile &file);
		void write(const std::string &filename);

		void print(std::ostream &o);
		bool prettyPrintInstruction(std::ostream &o, const Instruction &instr, unsigned int addr, int addressWidth);
	};
}

#endif