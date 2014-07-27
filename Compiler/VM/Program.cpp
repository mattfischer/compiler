#include "VM/Program.h"

#include "VM/OrcFile.h"

#include <iostream>
#include <iomanip>

namespace VM {

struct OrcSymbol {
	unsigned int name;
	unsigned int offset;
};

Program::Program()
{
	exportInfo = 0;
}

Program::Program(const OrcFile &file)
{
	exportInfo = 0;
	read(file);
}

Program::Program(const std::string &filename)
{
	exportInfo = 0;
	OrcFile file(filename);
	read(file);
}

void Program::read(const OrcFile &file)
{
	const OrcFile::Section *codeSection = file.section("code");
	instructions.resize(codeSection->data.size());
	std::memcpy(&instructions[0], &codeSection->data[0], codeSection->data.size());

	const OrcFile::Section *symbolsSection = file.section("symbols");
	const OrcFile::Section *symbolStringsSection = file.section("symbols.strings");
	for(unsigned int i=0; i<symbolsSection->data.size() / sizeof(OrcSymbol); i++) {
		const OrcSymbol *symbol = (OrcSymbol*)&symbolsSection->data[0] + i;
		std::string name = file.getString(symbolStringsSection, symbol->name);
		symbols[name] = symbol->offset;
	}

	const OrcFile::Section *exportInfoSection = file.section("export_info");
	const OrcFile::Section *exportInfoStringsSection = file.section("export_info.strings");
	exportInfo = new Front::ExportInfo(exportInfoSection->data, exportInfoStringsSection->data);
}

void Program::write(OrcFile &file)
{
	OrcFile::Section *codeSection = file.addSection("code");
	codeSection->data = instructions;

	OrcFile::Section *symbolStringsSection = file.addSection("symbols.strings");
	OrcFile::Section *symbolsSection = file.addSection("symbols");

	symbolsSection->data.resize(symbols.size() * sizeof(OrcSymbol));
	unsigned int s=0;
	for(std::map<std::string, int>::iterator it = symbols.begin(); it != symbols.end(); it++) {
		OrcSymbol *symbol = (OrcSymbol*)&symbolsSection->data[0] + s;
		s++;
		symbol->name = file.addString(symbolStringsSection, it->first);
		symbol->offset = it->second;
	}

	if(exportInfo) {
		OrcFile::Section *exportInfoSection = file.addSection("export_info");
		OrcFile::Section *exportInfoStringsSection = file.addSection("export_info.strings");

		exportInfoSection->data = exportInfo->data();
		exportInfoStringsSection->data = exportInfo->strings();
	}
}

void Program::write(const std::string &filename)
{
	OrcFile file;
	write(file);
	file.write(filename);
}

void Program::print(std::ostream &o)
{
	for(unsigned int i = 0; i < instructions.size(); i+=4) {
		VM::Instruction instr;
		for(std::map<std::string, int>::iterator it=symbols.begin(); it != symbols.end(); it++) {
			if(i == it->second) {
				o << it->first << ":" << std::endl;
				break;
			}
		}

		int addressWidth = 0;
		unsigned int size = (unsigned int)instructions.size();
		while(size > 0) {
			size >>= 4;
			addressWidth++;
		}

		o << "  0x" << std::setw(addressWidth) << std::setfill('0') << std::setbase(16) << i << ": ";
		for(int j=0; j<4; j++) {
			int d = 0;
			if(i + j < instructions.size()) {
				d = instructions[i + j];
			}
			o << std::setw(2) << std::setfill('0') << std::setbase(16) << d;
		}
		o << std::setbase(10);
		std::memcpy(&instr, &instructions[i], 4);
		o << "  " << instr << std::endl;
	}
}

}