#ifndef FRONT_EXPORT_INFO_H
#define FRONT_EXPORT_INFO_H

#include "Front/Types.h"
#include "Front/Scope.h"

#include <vector>
#include <map>

namespace Front {

class ExportInfo {
public:
	ExportInfo(Types &types, Scope &scope);
	ExportInfo(const std::vector<unsigned char> &data, const std::vector<unsigned char> &strings);

	void read(Types &types, Scope &scope);
	std::vector<std::string> typeNames();

	const std::vector<unsigned char> &data() { return mData; }
	const std::vector<unsigned char> &strings() { return mStrings; }

	enum class ExportItem {
		TypeDefinition,
		Symbol,
		StringBase
	};

private:
	std::vector<unsigned char> mData;
	std::vector<unsigned char> mStrings;

	std::map<std::string, unsigned char> mStringMap;

	std::string getString(unsigned int offset, unsigned int stringBase);
	unsigned char addString(const std::string &str);

	Type *readType(unsigned int &offset, Types &types, unsigned int stringBase);
	void bypassType(unsigned int &offset);
	void writeType(Type *type);

	Type *getType(const std::string &name, Types &types);
};

}

#endif
