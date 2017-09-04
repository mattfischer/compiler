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
	std::ifstream stream(filename.c_str(), std::ios_base::in | std::ios_base::binary);
	read(stream);
}

OrcFile::OrcFile()
{
	std::unique_ptr<Section> nameSection = std::make_unique<Section>();
	nameSection->name = addString(*nameSection, "$strings");
	mNameSection = 0;

	mSectionMap.insert(std::pair<std::string, std::reference_wrapper<Section>>("$strings", *nameSection));
	mSectionList.push_back(std::move(nameSection));
}

void OrcFile::write(std::ostream &stream)
{
	OrcHeader header;
	std::memcpy((char*)&header.magic, "ORC", 4);
	header.numSections = (unsigned int)mSectionList.size();
	header.nameSection = mNameSection;

	stream.write((char*)&header, sizeof(header));

	unsigned int offset = (unsigned int)(sizeof(OrcHeader) + mSectionList.size() * sizeof(OrcSectionHeader));
	for (const std::unique_ptr<Section> &section : mSectionList) {
		OrcSectionHeader sectionHeader;
		sectionHeader.name = section->name;
		sectionHeader.offset = offset;
		sectionHeader.size = (unsigned int)section->data.size();
		offset += (unsigned int)section->data.size();

		stream.write((char*)&sectionHeader, sizeof(sectionHeader));
	}

	for(const std::unique_ptr<Section> &section : mSectionList) {
		stream.write((char*)&section->data[0], (std::streamsize)section->data.size());
	}
}

void OrcFile::write(const std::string &filename)
{
	std::ofstream stream(filename.c_str(), std::ios_base::out | std::ios_base::binary);
	write(stream);
}

const OrcFile::Section *OrcFile::section(const std::string &name) const
{
	std::map<std::string, std::reference_wrapper<Section>>::const_iterator it = mSectionMap.find(name);
	if(it != mSectionMap.end()) {
		const Section &section = it->second;
		return &section;
	} else {
		return 0;
	}
}

OrcFile::Section *OrcFile::section(const std::string &name)
{
	std::map<std::string, std::reference_wrapper<Section>>::iterator it = mSectionMap.find(name);
	if(it != mSectionMap.end()) {
		Section &section = it->second;
		return &section;
	} else {
		return 0;
	}
}

unsigned int OrcFile::addString(Section &stringTable, const std::string &str)
{
	unsigned int offset = (unsigned int)stringTable.data.size();
	stringTable.data.resize(offset + str.size() + 1);
	std::memcpy(&stringTable.data[offset], str.c_str(), str.size());
	stringTable.data[offset + str.size()] = '\0';
	return offset;
}

std::string OrcFile::getString(const Section &stringTable, unsigned int offset)
{
	return std::string((char*)&stringTable.data[offset]);
}

OrcFile::Section &OrcFile::addSection(const std::string &name)
{
	std::unique_ptr<Section> section = std::make_unique<Section>();
	section->name = addString(*mSectionList[mNameSection], name);
	Section &ret = *section;
	mSectionMap.insert(std::pair<std::string, std::reference_wrapper<Section>>(name, *section));
	mSectionList.push_back(std::move(section));

	return ret;
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
		std::unique_ptr<Section> section = std::make_unique<Section>();
		section->data.resize(sectionHeaders[i].size);
		stream.seekg(sectionHeaders[i].offset);
		stream.read((char*)&section->data[0], sectionHeaders[i].size);
		section->name = sectionHeaders[i].name;
		mSectionList.push_back(std::move(section));
	}

	for(const std::unique_ptr<Section> &section : mSectionList) {
		mSectionMap.insert(std::pair<std::string, std::reference_wrapper<Section>>(getString(*mSectionList[mNameSection], section->name), *section));
	}
}

}
