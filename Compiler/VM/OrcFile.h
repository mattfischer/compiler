#ifndef VM_ORCFILE_H
#define VM_ORCFILE_H

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <memory>

namespace VM {

class OrcFile {
public:
	OrcFile();
	OrcFile(std::istream &stream);
	OrcFile(const std::string &filename);

	void write(std::ostream &stream);
	void write(const std::string &filename);

	struct Section {
		unsigned int name;
		std::vector<unsigned char> data;
	};

	const Section *section(const std::string &name) const;
	Section *section(const std::string &name);

	static unsigned int addString(Section &stringTable, const std::string &str);
	static std::string getString(const Section &stringTable, unsigned int offset);

	Section &addSection(const std::string &name);

private:
	std::vector<std::unique_ptr<Section>> mSectionList;
	std::map<std::string, std::reference_wrapper<Section>> mSectionMap;
	unsigned int mNameSection;

	void read(std::istream &stream);
};

}
#endif
