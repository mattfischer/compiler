#include "VM/Program.h"

#include <iostream>
#include <iomanip>

namespace VM {

struct OrcHeader {
	char magic[4];
	unsigned int numSections;
	unsigned int nameSection;
};

struct OrcSectionHeader {
	unsigned int name;
	unsigned int offset;
	unsigned int size;
};

struct OrcSymbol {
	unsigned int name;
	unsigned int offset;
};

struct Section {
	std::string name;
	std::vector<unsigned char> data;

	Section() {}
	Section(const std::string &_name) : name(_name) {}
};

std::string readString(Section *stringTable, int offset)
{
	return std::string((char*)&stringTable->data[offset]);
}

Program::Program()
{
}

Program::Program(std::istream &stream)
{
	stream.seekg(0);
	OrcHeader header;
	stream.read((char*)&header, sizeof(header));

	OrcSectionHeader *sectionHeaders = new OrcSectionHeader[header.numSections];
	stream.read((char*)sectionHeaders, sizeof(OrcSectionHeader) * header.numSections);

	std::vector<Section*> sections;
	for(unsigned int i=0; i<header.numSections; i++) {
		Section *section = new Section;
		section->data.resize(sectionHeaders[i].size);
		stream.seekg(sectionHeaders[i].offset);
		stream.read((char*)&section->data[0], sectionHeaders[i].size);

		sections.push_back(section);
	}

	std::map<std::string, Section*> sectionMap;
	for(unsigned int i=0; i<header.numSections; i++) {
		std::string name = readString(sections[header.nameSection], sectionHeaders[i].name);
		sections[i]->name = name;
		sectionMap[name] = sections[i];
	}

	Section *codeSection = sectionMap["code"];
	instructions.resize(codeSection->data.size());
	std::memcpy(&instructions[0], &codeSection->data[0], codeSection->data.size());

	Section *symbolsSection = sectionMap["symbols"];
	Section *symbolStringsSection = sectionMap["symbols.strings"];
	for(unsigned int i=0; i<symbolsSection->data.size() / sizeof(OrcSymbol); i++) {
		OrcSymbol *symbol = (OrcSymbol*)&symbolsSection->data[0] + i;
		std::string name = readString(symbolStringsSection, symbol->name);
		symbols[name] = symbol->offset;
	}

	for(unsigned int i=0; i<sections.size(); i++) {
		delete sections[i];
	}
	delete[] sectionHeaders;
}

unsigned int addString(Section *stringTable, const std::string &str)
{
	unsigned int offset = (unsigned int)stringTable->data.size();
	stringTable->data.resize(stringTable->data.size() + str.size() + 1);
	std::memcpy(&stringTable->data[offset], str.c_str(), str.size());
	stringTable->data[offset + str.size()] = '\0';
	return offset;
}

void Program::write(std::ostream &o)
{
	std::vector<Section*> sections;

	Section *codeSection = new Section("code");
	codeSection->data.resize(instructions.size());
	sections.push_back(codeSection);
	std::memcpy(&codeSection->data[0], &instructions[0], instructions.size());

	Section *symbolsSection = new Section("symbols");
	sections.push_back(symbolsSection);
	Section *symbolStringsSection = new Section("symbols.strings");
	sections.push_back(symbolStringsSection);
	symbolsSection->data.resize(symbols.size() * sizeof(OrcSymbol));
	unsigned int s=0;
	for(std::map<std::string, int>::iterator it = symbols.begin(); it != symbols.end(); it++) {
		OrcSymbol *symbol = (OrcSymbol*)&symbolsSection->data[0] + s;
		s++;
		symbol->name = addString(symbolStringsSection, it->first);
		symbol->offset = it->second;
	}

	Section *stringTable = new Section("strings");
	sections.push_back(stringTable);

	std::vector<OrcSectionHeader> sectionHeaders;
	sectionHeaders.resize(sections.size());
	for(unsigned int i=0; i<sections.size(); i++) {
		sectionHeaders[i].name = addString(stringTable, sections[i]->name);
	}

	for(unsigned int i=0; i<sectionHeaders.size(); i++) {
		sectionHeaders[i].name = addString(stringTable, sections[i]->name);
	}

	unsigned int stringTableNumber;
	unsigned int offset = (unsigned int)(sizeof(OrcHeader) + sections.size() * sizeof(OrcSectionHeader));
	for(unsigned int i=0; i<sectionHeaders.size(); i++) {
		sectionHeaders[i].offset = offset;
		sectionHeaders[i].size = (unsigned int)sections[i]->data.size();
		offset += sectionHeaders[i].size;

		if(sections[i] == stringTable) {
			stringTableNumber = i;
		}
	}

	OrcHeader header;
	std::memcpy((char*)&header.magic, "ORC", 4);
	header.numSections = (unsigned int)sections.size();
	header.nameSection = stringTableNumber;
	o.write((char*)&header, sizeof(header));
	o.write((char*)&sectionHeaders[0], (unsigned int)(sizeof(OrcSectionHeader) * sectionHeaders.size()));
	for(unsigned int i=0; i<sections.size(); i++) {
		o.write((char*)&sections[i]->data[0], (unsigned int)sections[i]->data.size());
	}
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