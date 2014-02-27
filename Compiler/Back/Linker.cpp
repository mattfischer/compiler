#include "Back/Linker.h"

#include <sstream>

namespace Back {

Linker::Linker()
{
	mError = false;
}

VM::Program *Linker::link(const std::vector<VM::Program*> &programs)
{
	VM::Program *linked = new VM::Program;

	int size = 0;
	for(unsigned int i=0; i<programs.size(); i++) {
		size += (int)programs[i]->instructions.size();
	}
	linked->instructions.resize(size);

	int offset = 0;
	for(unsigned int i=0; i<programs.size(); i++) {
		VM::Program *program = programs[i];
		std::memcpy(&linked->instructions[offset], &program->instructions[0], program->instructions.size());

		for(std::map<std::string, int>::iterator it = program->symbols.begin(); it != program->symbols.end(); it++) {
			linked->symbols[it->first] = it->second + offset;
		}

		for(std::map<int, std::string>::iterator it = program->imports.begin(); it != program->imports.end(); it++) {
			linked->imports[it->first + offset] = it->second;
		}

		offset += (int)program->instructions.size();
	}

	for(std::map<int, std::string>::iterator it = linked->imports.begin(); it != linked->imports.end(); it++) {
		int offset = it->first;
		const std::string &symbol = it->second;
		VM::Instruction instr;

		std::memcpy(&instr, &linked->instructions[offset], 4);
		if(linked->symbols.find(symbol) == linked->symbols.end()) {
			mError = true;
			std::stringstream s;
			s << "Undefined symbol " << symbol;
			mErrorMessage = s.str();
			delete linked;
			return 0;
		} else {
			int symbolOffset = linked->symbols[symbol];
			instr.one.imm = (symbolOffset - offset) / 4;
			std::memcpy(&linked->instructions[offset], &instr, 4);
		}
	}

	linked->imports.clear();
	return linked;
}

}