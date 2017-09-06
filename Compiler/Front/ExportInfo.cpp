#include "Front/ExportInfo.h"

namespace Front {

enum class ExportType {
	Simple,
	Procedure,
	Array
};

enum class ExportDefinition {
	Class,
	Struct
};

enum ExportMember {
	Normal,
	Virtual,
	Static
};

ExportInfo::ExportInfo(Types &types, Scope &scope)
{
	for(Type *type : types.types()) {
		switch(type->kind) {
			case Type::Kind::Struct:
				{
					TypeStruct *typeStruct = (TypeStruct*)type;
					mData.push_back((unsigned char)ExportItem::TypeDefinition);
					mData.push_back((unsigned char)ExportDefinition::Struct);
					mData.push_back(addString(type->name));
					mData.push_back((unsigned char)typeStruct->members.size());
					for(TypeStruct::Member &member : typeStruct->members) {
						writeType(member.type);
						mData.push_back(addString(member.name));
					}
					break;
				}

			case Type::Kind::Class:
				{
					TypeStruct *typeStruct = (TypeStruct*)type;
					mData.push_back((unsigned char)ExportItem::TypeDefinition);
					mData.push_back((unsigned char)ExportDefinition::Class);
					mData.push_back(addString(type->name));
					if(typeStruct->parent) {
						mData.push_back(addString(typeStruct->parent->name));
					} else {
						mData.push_back((unsigned char)0xff);
					}
					mData.push_back((unsigned char)typeStruct->members.size());
					for(TypeStruct::Member &member : typeStruct->members) {
						if(member.qualifiers & TypeStruct::Member::QualifierVirtual) {
							mData.push_back((unsigned char)ExportMember::Virtual);
						} else if(member.qualifiers & TypeStruct::Member::QualifierStatic) {
							mData.push_back((unsigned char)ExportMember::Static);
						} else {
							mData.push_back((unsigned char)ExportMember::Normal);
						}

						writeType(member.type);
						mData.push_back(addString(member.name));
					}
					break;
				}
		}
	}

	for(Symbol *symbol : scope.symbols()) {
		mData.push_back((unsigned char)ExportItem::Symbol);
		writeType(symbol->type);
		mData.push_back(addString(symbol->name));
	}
}

ExportInfo::ExportInfo(const std::vector<unsigned char> &data, const std::vector<unsigned char> &strings)
: mData(data), mStrings(strings)
{
}

void ExportInfo::read(Types &types, Scope &scope)
{
	unsigned int offset = 0;
	unsigned int stringBase = 0;

	while(offset < mData.size()) {
		ExportItem exportItem = (ExportItem)mData[offset];
		offset++;
		switch(exportItem) {
			case ExportItem::TypeDefinition:
				{
					ExportDefinition exportDefinition = (ExportDefinition)mData[offset];
					offset++;
					std::string name = getString(mData[offset], stringBase);
					offset++;

					std::unique_ptr<Type> type;
					switch(exportDefinition) {
						case ExportDefinition::Class:
							{
								std::unique_ptr<TypeStruct> typeStruct = std::make_unique<TypeStruct>(Type::Kind::Class, name);
								if(mData[offset] == 0xff) {
									typeStruct->parent = 0;
								} else {
									typeStruct->parent = (TypeStruct*)getType(getString(mData[offset], stringBase), types);
								}
								typeStruct->constructor = 0;
								offset++;

								int numMembers = mData[offset];
								offset++;
								for(int i=0; i<numMembers; i++) {
									ExportMember exportMember = (ExportMember)mData[offset];
									offset++;
									Type *memberType = readType(offset, types, stringBase);
									std::string memberName = getString(mData[offset], stringBase);
									offset++;
									unsigned int qualifiers = 0;
									switch(exportMember) {
										case ExportMember::Virtual: 
											qualifiers = TypeStruct::Member::QualifierVirtual;
											break;
										case ExportMember::Static:
											qualifiers = TypeStruct::Member::QualifierStatic;
											break;
									}

									typeStruct->addMember(memberType, memberName, qualifiers);

									if(memberName == typeStruct->name) {
										typeStruct->constructor = (TypeProcedure*)memberType;
									}
								}
								type = std::move(typeStruct);
								break;
							}

						case ExportDefinition::Struct:
							{
								std::unique_ptr<TypeStruct> typeStruct = std::make_unique<TypeStruct>(Type::Kind::Struct, name);
								typeStruct->constructor = 0;
								int numMembers = mData[offset];
								offset++;
								for(int i=0; i<numMembers; i++) {
									Type *memberType = readType(offset, types, stringBase);
									std::string memberName = getString(mData[offset], stringBase);
									offset++;
									typeStruct->addMember(memberType, memberName, false);
								}
								type = std::move(typeStruct);
								break;
							}
					}
					types.registerType(std::move(type));
					break;
				}

			case ExportItem::Symbol:
				{
					Type *type = readType(offset, types, stringBase);
					std::string name = getString(mData[offset], stringBase);
					offset++;
					scope.addSymbol(new Symbol(type, name));
					break;
				}

			case ExportItem::StringBase:
				{
					stringBase = mData[offset];
					offset++;
					break;
				}
		}
	}
}

std::vector<std::string> ExportInfo::typeNames()
{
	std::vector<std::string> typeNames;

	unsigned int offset = 0;
	unsigned int stringBase = 0;
	while(offset < mData.size()) {
		ExportItem exportItem = (ExportItem)mData[offset];
		offset++;
		switch(exportItem) {
			case ExportItem::TypeDefinition:
				{
					ExportDefinition exportDefinition = (ExportDefinition)mData[offset];
					offset++;
					std::string name = getString(mData[offset], stringBase);
					offset++;

					switch(exportDefinition) {
						case ExportDefinition::Class:
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

						case ExportDefinition::Struct:
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

			case ExportItem::Symbol:
				{
					bypassType(offset);
					offset++;
					break;
				}

			case ExportItem::StringBase:
				{
					stringBase = mData[offset];
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

std::string ExportInfo::getString(unsigned int offset, unsigned int stringBase)
{
	return std::string((char*)&mStrings[stringBase + offset]);
}

Type *ExportInfo::readType(unsigned int &offset, Types &types, unsigned int stringBase)
{
	ExportType exportType = (ExportType)mData[offset];
	offset++;

	Type *type;
	switch(exportType) {
		case ExportType::Simple:
			{
				std::string name = getString(mData[offset], stringBase);
				offset++;
				type = getType(name, types);
				break;
			}

		case ExportType::Procedure:
			{
				Type *returnType = readType(offset, types, stringBase);
				int numArguments = mData[offset];
				offset++;
				std::vector<Type*> argumentTypes;
				for(int i=0; i<numArguments; i++) {
					argumentTypes.push_back(readType(offset, types, stringBase));
				}
				type = new TypeProcedure(returnType, argumentTypes);
				break;
			}

		case ExportType::Array:
			{
				Type *baseType = readType(offset, types, stringBase);
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
		case ExportType::Simple:
			{
				offset++;
				break;
			}

		case ExportType::Procedure:
			{
				bypassType(offset);
				int numArguments = mData[offset];
				offset++;
				for(int i=0; i<numArguments; i++) {
					bypassType(offset);
				}
				break;
			}

		case ExportType::Array:
			{
				bypassType(offset);
				break;
			}
	}
}

void ExportInfo::writeType(Type *type)
{
	switch(type->kind) {
		case Type::Kind::Intrinsic:
		case Type::Kind::Struct:
		case Type::Kind::Class:
			mData.push_back((unsigned char)ExportType::Simple);
			mData.push_back(addString(type->name));
			break;

		case Type::Kind::Procedure:
			{
				TypeProcedure *typeProcedure = (TypeProcedure*)type;
				mData.push_back((unsigned char)ExportType::Procedure);
				writeType(typeProcedure->returnType);
				mData.push_back((unsigned char)typeProcedure->argumentTypes.size());
				for(Type *argumentType : typeProcedure->argumentTypes) {
					writeType(argumentType);
				}
				break;
			}

		case Type::Kind::Array:
			{
				TypeArray *typeArray = (TypeArray*)type;
				mData.push_back((unsigned char)ExportType::Array);
				writeType(typeArray->baseType);
				break;
			}
	}
}

Type *ExportInfo::getType(const std::string &name, Types &types)
{
	Type *type = types.findType(name);
	if(!type) {
		type = new TypeDummy(name, "");
	}

	return type;
}

}
