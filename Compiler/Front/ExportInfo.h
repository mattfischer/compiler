#ifndef FRONT_EXPORT_INFO_H
#define FRONT_EXPORT_INFO_H

#include "Front/Types.h"
#include "Front/Scope.h"

#include <vector>
#include <map>

namespace Front {

class ExportInfo {
public:
	ExportInfo(Types *types, Scope *scope);
	ExportInfo(const std::vector<unsigned char> &data, const std::vector<unsigned char> &strings);

	const std::vector<unsigned char> &data() { return mData; }
	const std::vector<unsigned char> &strings() { return mStrings; }

	Types *types();
	Scope *scope();

private:
	std::vector<unsigned char> mData;
	std::vector<unsigned char> mStrings;
	Types *mTypes;
	Scope *mScope;

	std::map<std::string, unsigned char> mStringMap;

	void parse();
	unsigned char addString(const std::string &str);
	void writeType(Type *type);
};

}

#endif
