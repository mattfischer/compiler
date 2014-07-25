#include "Front/ExportInfo.h"

namespace Front {

enum ExportItem {
	ExportItemTypeDefinition,
	ExportItemSymbol
};

enum ExportType {
	ExportTypeSimple,
	ExportTypeProcedure,
	ExportTypeArray
};

enum ExportDefinition {
	ExportDefinitionClass,
	ExportDefinitionStruct
};

enum ExportMember {
	ExportMemberNormal,
	ExportMemberVirtual
};

ExportInfo::ExportInfo(Types *types, Scope *scope)
{
	for(unsigned int i=0; i<types->types().size(); i++) {
		Type *type = types->types()[i];

		switch(type->type) {
			case Type::TypeStruct:
				{
					TypeStruct *typeStruct = (TypeStruct*)type;
					mData.push_back((unsigned char)ExportItemTypeDefinition);
					mData.push_back((unsigned char)ExportDefinitionStruct);
					mData.push_back(addString(type->name));
					mData.push_back((unsigned char)typeStruct->members.size());
					for(unsigned int j=0; j<typeStruct->members.size(); j++) {
						writeType(typeStruct->members[j].type);
						mData.push_back(addString(typeStruct->members[j].name));
					}
					break;
				}

			case Type::TypeClass:
				{
					TypeStruct *typeStruct = (TypeStruct*)type;
					mData.push_back((unsigned char)ExportItemTypeDefinition);
					mData.push_back((unsigned char)ExportDefinitionClass);
					mData.push_back(addString(type->name));
					if(typeStruct->parent) {
						mData.push_back(addString(typeStruct->parent->name));
					} else {
						mData.push_back((unsigned char)0xff);
					}
					mData.push_back((unsigned char)typeStruct->members.size());
					for(unsigned int j=0; j<typeStruct->members.size(); j++) {
						if(typeStruct->members[j].virtualFunction) {
							mData.push_back((unsigned char)ExportMemberVirtual);
						} else {
							mData.push_back((unsigned char)ExportMemberNormal);
						}
						writeType(typeStruct->members[j].type);
						mData.push_back(addString(typeStruct->members[j].name));
					}
					break;
				}
		}
	}

	for(unsigned int i=0; i<scope->symbols().size(); i++) {
		Symbol *symbol = scope->symbols()[i];
		mData.push_back((unsigned char)ExportItemSymbol);
		writeType(symbol->type);
		mData.push_back(addString(symbol->name));
	}
}

ExportInfo::ExportInfo(const std::vector<unsigned char> &data, const std::vector<unsigned char> &strings)
: mData(data), mStrings(strings)
{
	mTypes = 0;
	mScope = 0;
}

Types *ExportInfo::types()
{
	if(!mTypes) {
		parse();
	}

	return mTypes;
}

Scope *ExportInfo::scope()
{
	if(!mScope) {
		parse();
	}

	return mScope;
}

void ExportInfo::parse()
{
}

unsigned char ExportInfo::addString(const std::string &str)
{
	std::map<std::string, unsigned char>::iterator it;

	it = mStringMap.find(str);
	if(it == mStringMap.end()) {
		unsigned char offset = (unsigned char)mStrings.size();
		mStrings.resize(offset + str.size() + 1);
		std::memcpy(&mStrings[offset], str.c_str(), str.size());
		mStrings[offset + str.size()] = '\0';
		mStringMap[str] = offset;
		return offset;
	} else {
		return it->second;
	}
}

void ExportInfo::writeType(Type *type)
{
	switch(type->type) {
		case Type::TypeIntrinsic:
		case Type::TypeStruct:
		case Type::TypeClass:
			mData.push_back((unsigned char)ExportTypeSimple);
			mData.push_back(addString(type->name));
			break;

		case Type::TypeProcedure:
			{
				TypeProcedure *typeProcedure = (TypeProcedure*)type;
				mData.push_back((unsigned char)ExportTypeProcedure);
				writeType(typeProcedure->returnType);
				mData.push_back((unsigned char)typeProcedure->argumentTypes.size());
				for(unsigned int i=0; i<typeProcedure->argumentTypes.size(); i++) {
					writeType(typeProcedure->argumentTypes[i]);
				}
				break;
			}

		case Type::TypeArray:
			{
				TypeArray *typeArray = (TypeArray*)type;
				mData.push_back((unsigned char)ExportTypeArray);
				writeType(typeArray->baseType);
				break;
			}
	}
}

}
