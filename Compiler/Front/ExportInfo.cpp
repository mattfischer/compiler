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
}

void ExportInfo::read(Types *types, Scope *scope)
{
	unsigned int offset = 0;
	while(offset < mData.size()) {
		ExportItem exportItem = (ExportItem)mData[offset];
		offset++;
		switch(exportItem) {
			case ExportItemTypeDefinition:
				{
					ExportDefinition exportDefinition = (ExportDefinition)mData[offset];
					offset++;
					std::string name = getString(mData[offset]);
					offset++;

					Type *type;
					switch(exportDefinition) {
						case ExportDefinitionClass:
							{
								TypeStruct *typeStruct = new TypeStruct(Type::TypeClass, name);
								if(mData[offset] == 0xff) {
									typeStruct->parent = 0;
								} else {
									typeStruct->parent = (TypeStruct*)getType(getString(mData[offset]), types);
								}
								typeStruct->constructor = 0;
								offset++;

								int numMembers = mData[offset];
								offset++;
								for(int i=0; i<numMembers; i++) {
									ExportMember exportMember = (ExportMember)mData[offset];
									offset++;
									Type *memberType = readType(offset, types);
									std::string memberName = getString(mData[offset]);
									offset++;
									typeStruct->addMember(memberType, memberName, exportMember == ExportMemberVirtual);

									if(memberName == typeStruct->name) {
										typeStruct->constructor = (TypeProcedure*)memberType;
									}
								}
								type = typeStruct;
								break;
							}

						case ExportDefinitionStruct:
							{
								TypeStruct *typeStruct = new TypeStruct(Type::TypeStruct, name);
								typeStruct->constructor = 0;
								int numMembers = mData[offset];
								offset++;
								for(int i=0; i<numMembers; i++) {
									Type *memberType = readType(offset, types);
									std::string memberName = getString(mData[offset]);
									offset++;
									typeStruct->addMember(memberType, memberName, false);
								}
								type = typeStruct;
								break;
							}
					}
					types->registerType(type);
					break;
				}

			case ExportItemSymbol:
				{
					Type *type = readType(offset, types);
					std::string name = getString(mData[offset]);
					offset++;
					scope->addSymbol(new Symbol(type, name));
					break;
				}
		}
	}
}

std::vector<std::string> ExportInfo::typeNames()
{
	std::vector<std::string> typeNames;

	unsigned int offset = 0;
	while(offset < mData.size()) {
		ExportItem exportItem = (ExportItem)mData[offset];
		offset++;
		switch(exportItem) {
			case ExportItemTypeDefinition:
				{
					ExportDefinition exportDefinition = (ExportDefinition)mData[offset];
					offset++;
					std::string name = getString(mData[offset]);
					offset++;

					switch(exportDefinition) {
						case ExportDefinitionClass:
							{
								offset++;
								int numMembers = mData[offset];
								offset++;
								for(int i=0; i<numMembers; i++) {
									offset++;
									bypassType(offset);
									offset++;
								}
								break;
							}

						case ExportDefinitionStruct:
							{
								int numMembers = mData[offset];
								offset++;
								for(int i=0; i<numMembers; i++) {
									bypassType(offset);
									offset++;
								}
								break;
							}
					}
					typeNames.push_back(name);
					break;
				}

			case ExportItemSymbol:
				{
					bypassType(offset);
					offset++;
					break;
				}
		}
	}

	return typeNames;
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

std::string ExportInfo::getString(unsigned int offset)
{
	return std::string((char*)&mStrings[offset]);
}

Type *ExportInfo::readType(unsigned int &offset, Types *types)
{
	ExportType exportType = (ExportType)mData[offset];
	offset++;

	Type *type;
	switch(exportType) {
		case ExportTypeSimple:
			{
				std::string name = getString(mData[offset]);
				offset++;
				type = getType(name, types);
				break;
			}

		case ExportTypeProcedure:
			{
				Type *returnType = readType(offset, types);
				int numArguments = mData[offset];
				offset++;
				std::vector<Type*> argumentTypes;
				for(int i=0; i<numArguments; i++) {
					argumentTypes.push_back(readType(offset, types));
				}
				type = new TypeProcedure(returnType, argumentTypes);
				break;
			}

		case ExportTypeArray:
			{
				Type *baseType = readType(offset, types);
				type = new TypeArray(baseType);
				break;
			}
	}

	return type;
}

void ExportInfo::bypassType(unsigned int &offset)
{
	ExportType exportType = (ExportType)mData[offset];
	offset++;

	switch(exportType) {
		case ExportTypeSimple:
			{
				offset++;
				break;
			}

		case ExportTypeProcedure:
			{
				bypassType(offset);
				int numArguments = mData[offset];
				offset++;
				for(int i=0; i<numArguments; i++) {
					bypassType(offset);
				}
				break;
			}

		case ExportTypeArray:
			{
				bypassType(offset);
				break;
			}
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

Type *ExportInfo::getType(const std::string &name, Types *types)
{
	Type *type = types->findType(name);
	if(!type) {
		type = new TypeDummy(name, "");
	}

	return type;
}

}
