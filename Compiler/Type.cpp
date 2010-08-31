#include "Type.h"

TypeIntrinsic _TypeInt(TypeIntrinsic::Int, "int");
Type *TypeInt = &_TypeInt;

Type _TypeNone(Type::TypeNone, "");
Type *TypeNone = &_TypeNone;

std::vector<Type*> Type::sTypes;

void Type::init()
{
	sTypes.push_back(::TypeNone);
	sTypes.push_back(::TypeInt);
}

Type *Type::find(const std::string &name)
{
	for(int i=0; i<sTypes.size(); i++) {
		if(sTypes[i]->name == name) {
			return sTypes[i];
		}
	}

	return NULL;
}