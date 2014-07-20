#include "VM/OrcFile.h"

#include <fstream>

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

OrcFile::OrcFile(std::istream &stream)
{
	read(stream);
}

OrcFile::OrcFile(const std::string &filename)
{
	std::ifstream stream(filename.c_str());
	read(stream);
}

OrcFile::OrcFile()
{
	Section *nameSection = new Section;
	nameSection->name = addString(nameSection, "$strings");
	mNameSection = 0;

	mSectionList.push_back(nameSection);
	mSectionMap["$strings"] = nameSection;
}

OrcFile::~OrcFile()
{
	for(unsigned int i=0; i<mSectionList.size(); i++) {
		delete mSectionList[i];
	}
}

void OrcFile::write(std::ostream &stream)
{
	OrcHeader header;
	std::memcpy((char*)&header.magic, "ORC", 4);
	header.numSections = (unsigned int)mSectionList.size();
	header.nameSection = mNameSection;

	stream.write((char*)&header, sizeof(header));

	unsigned int offset = (unsigned int)(sizeof(OrcHeader) + mSectionList.size() * sizeof(OrcSectionHeader));
	for(unsigned int i=0; i<mSectionList.size(); i++) {
		OrcSectionHeader sectionHeader;
		sectionHeader.name = mSectionList[i]->name;
		sectionHeader.offset = offset;
		sectionHeader.size = (unsigned int)mSectionList[i]->data.size();
		offset += (unsigned int)mSectionList[i]->data.size();

		stream.write((char*)&sectionHeader, sizeof(sectionHeader));
	}

	for(unsigned int i=0; i<mSectionList.size(); i++) {
		stream.write((char*)&mSectionList[i]->data[0], (std::streamsize)mSectionList[i]->data.size());
	}
}

void OrcFile::write(const std::string &filename)
{
	std::ofstream stream(filename.c_str());
	write(stream);
}

const OrcFile::Section *OrcFile::section(const std::string &name) const
{
	std::map<std::string, Section*>::const_iterator it = mSectionMap.find(name);
	if(it != mSectionMap.end()) {
		return it->second;
	} else {
		return 0;
	}
}

OrcFile::Section *OrcFile::section(const std::string &name)
{
	std::map<std::string, Section*>::iterator it = mSectionMap.find(name);
	if(it != mSectionMap.end()) {
		return it->second;
	} else {
		return 0;
	}
}

unsigned int OrcFile::addString(Section *stringTable, const std::string &str)
{
	unsigned int offset = (unsigned int)stringTable->data.size();
	stringTable->data.resize(offset + str.size() + 1);
	std::memcpy(&stringTable->data[offset], str.c_str(), str.size());
	stringTable->data[offset + str.size()] = '\0';
	return offset;
}

std::string OrcFile::getString(const Section *stringTable, unsigned int offset)
{
	return std::string((char*)&stringTable->data[offset]);
}

OrcFile::Section *OrcFile::addSection(const std::string &name)
{
	Section *section = new Section;
	section->name = addString(mSectionList[mNameSection], name);
	mSectionList.push_back(section);
	mSectionMap[name] = section;

	return section;
}

void OrcFile::read(std::istream &stream)
{
	stream.seekg(0);
	OrcHeader header;
	stream.read((char*)&header, sizeof(header));

	mNameSection = header.nameSection;

	OrcSectionHeader *sectionHeaders = new OrcSectionHeader[header.numSections];
	stream.read((char*)sectionHeaders, sizeof(OrcSectionHeader) * header.numSections);

	for(unsigned int i=0; i<header.numSections; i++) {
		Section *section = new Section;
		section->data.resize(sectionHeaders[i].size);
		stream.seekg(sectionHeaders[i].offset);
		stream.read((char*)&section->data[0], sectionHeaders[i].size);
		section->name = sectionHeaders[i].name;
		mSectionList.push_back(section);
	}

	for(unsigned int i=0; i<mSectionList.size(); i++) {
		mSectionMap[getString(mSectionList[mNameSection], mSectionList[i]->name)] = mSectionList[i];
	}
}

}
