#include "Linker.h"

#include <sstream>

/*!
 * \brief Constructor
 */
Linker::Linker()
{
	mError = false;
}

/*!
 * \brief Link programs together
 * \param programs List of programs
 * \return Linked program
 */
VM::Program *Linker::link(const std::vector<VM::Program*> &programs)
{
	VM::Program *linked = new VM::Program;
	std::vector<VM::Program::Relocation> relocations;

	// Calculate total program size
	int size = 0;
	for(unsigned int i=0; i<programs.size(); i++) {
		size += (int)programs[i]->instructions.size();
	}
	linked->instructions.resize(size);

	// Concatenate programs together into linked program
	int offset = 0;
	for(unsigned int i=0; i<programs.size(); i++) {
		VM::Program *program = programs[i];
		std::memcpy(&linked->instructions[offset], &program->instructions[0], program->instructions.size());

		for(std::map<std::string, int>::iterator it = program->symbols.begin(); it != program->symbols.end(); it++) {
			linked->symbols[it->first] = it->second + offset;
		}

		for(unsigned int j=0; j<program->relocations.size(); j++) {
			VM::Program::Relocation relocation = program->relocations[j];
			relocation.offset += offset;
			relocations.push_back(relocation);
		}

		offset += (int)program->instructions.size();
	}

	// Resolve symbol references in program
	for(unsigned int i=0; i<relocations.size(); i++) {
		if(linked->symbols.find(relocations[i].symbol) == linked->symbols.end()) {
			mError = true;
			std::stringstream s;
			s << "Undefined symbol " << relocations[i].symbol;
			mErrorMessage = s.str();
			delete linked;
			return 0;
		}

		switch(relocations[i].type) {
			case VM::Program::Relocation::TypeAbsolute:
				{
					unsigned long address = linked->symbols[relocations[i].symbol];
					std::memcpy(&linked->instructions[relocations[i].offset], &address, 4);
					break;
				}

			case VM::Program::Relocation::TypeCall:
				{
					VM::Instruction instr;
					std::memcpy(&instr, &linked->instructions[relocations[i].offset], 4);
					int symbolOffset = linked->symbols[relocations[i].symbol];
					instr.one.imm = (symbolOffset - relocations[i].offset) / 4;
					std::memcpy(&linked->instructions[relocations[i].offset], &instr, 4);
					break;
				}

			case VM::Program::Relocation::TypeAddPCRel:
				{
					VM::Instruction instr;
					std::memcpy(&instr, &linked->instructions[relocations[i].offset], 4);
					int symbolOffset = linked->symbols[relocations[i].symbol];
					instr.two.imm = symbolOffset - relocations[i].offset;
					std::memcpy(&linked->instructions[relocations[i].offset], &instr, 4);
					break;
				}
		}
	}

	linked->exportInfo = programs[0]->exportInfo;

	return linked;
}