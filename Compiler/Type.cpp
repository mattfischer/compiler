#include "Type.h"

TypeIntrinsic _TypeInt(TypeIntrinsic::Int, "int", 4);
Type *TypeInt = &_TypeInt;

TypeIntrinsic _TypeBool(TypeIntrinsic::Int, "bool", 4);
Type *TypeBool = &_TypeBool;

Type _TypeNone(Type::TypeNone, "", 0);
Type *TypeNone = &_TypeNone;

std::vector<Type*> Type::sTypes;

void Type::init()
{
	sTypes.push_back(::TypeNone);
	sTypes.push_back(::TypeInt);
	sTypes.push_back(::TypeBool);
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