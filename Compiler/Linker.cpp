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
std::unique_ptr<VM::Program> Linker::link(const std::vector<std::reference_wrapper<const VM::Program>> &programs)
{
	std::unique_ptr<VM::Program> linked = std::make_unique<VM::Program>();
	std::vector<VM::Program::Relocation> relocations;

	std::vector<unsigned char> exportInfoData;
	std::vector<unsigned char> exportInfoStringData;

	// Concatenate programs together into linked program
	int offset = 0;
	for(unsigned int i=0; i<programs.size(); i++) {
		const VM::Program &program = programs[i];

		if(program.instructions.size() > 0) {
			linked->instructions.resize(linked->instructions.size() + program.instructions.size());
			std::memcpy(&linked->instructions[offset], &program.instructions[0], program.instructions.size());
		}

		for(std::map<std::string, int>::const_iterator it = program.symbols.begin(); it != program.symbols.end(); it++) {
			linked->symbols[it->first] = it->second + offset;
		}

		for(unsigned int j=0; j<program.relocations.size(); j++) {
			VM::Program::Relocation relocation = program.relocations[j];
			relocation.offset += offset;
			relocations.push_back(relocation);
		}

		offset += (int)program.instructions.size();

		unsigned int exportInfoOffset = (unsigned int)exportInfoData.size();
		unsigned int exportInfoStringOffset = (unsigned int)exportInfoStringData.size();

		if(program.exportInfo && program.exportInfo->data().size() > 0) {
			exportInfoData.resize(exportInfoOffset + program.exportInfo->data().size() + 2);
			exportInfoData[exportInfoOffset] = (unsigned char)Front::ExportInfo::ExportItem::StringBase;
			exportInfoData[exportInfoOffset + 1] = exportInfoStringOffset;
			std::memcpy(&exportInfoData[exportInfoOffset + 2], &program.exportInfo->data()[0], program.exportInfo->data().size());

			exportInfoStringData.resize(exportInfoStringOffset + program.exportInfo->strings().size());
			std::memcpy(&exportInfoStringData[exportInfoStringOffset], &program.exportInfo->strings()[0], program.exportInfo->strings().size());
		}
	}

	// Resolve symbol references in program
	for(unsigned int i=0; i<relocations.size(); i++) {
		if(linked->symbols.find(relocations[i].symbol) == linked->symbols.end()) {
			linked->relocations.push_back(relocations[i]);
		} else {
			switch(relocations[i].type) {
				case VM::Program::Relocation::Type::Absolute:
					{
						unsigned long address = linked->symbols[relocations[i].symbol];
						std::memcpy(&linked->instructions[relocations[i].offset], &address, 4);
						break;
					}

				case VM::Program::Relocation::Type::Call:
					{
						VM::Instruction instr;
						std::memcpy(&instr, &linked->instructions[relocations[i].offset], 4);
						int symbolOffset = linked->symbols[relocations[i].symbol];
						instr.one.imm = (symbolOffset - relocations[i].offset) / 4;
						std::memcpy(&linked->instructions[relocations[i].offset], &instr, 4);
						break;
					}

				case VM::Program::Relocation::Type::AddPCRel:
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
	}

	linked->exportInfo = std::make_unique<Front::ExportInfo>(exportInfoData, exportInfoStringData);

	return linked;
}