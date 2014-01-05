#include "Type.h"

namespace Front {
	TypeIntrinsic _TypeInt(TypeIntrinsic::Int, "int", 4);
	Type *TypeInt = &_TypeInt;

	TypeIntrinsic _TypeBool(TypeIntrinsic::Int, "bool", 4);
	Type *TypeBool = &_TypeBool;

	Type _TypeNone(Type::TypeNone, "", 0);
	Type *TypeNone = &_TypeNone;

	std::vector<Type*> Type::sTypes;

	void Type::init() {
		sTypes.push_back(Front::TypeNone);
		sTypes.push_back(Front::TypeInt);
		sTypes.push_back(Front::TypeBool);
	}

	class TypeInit {
	public:
		TypeInit() { Type::init(); }
	};
	TypeInit init;

	Type *Type::find(const std::string &name)
	{
		for(unsigned int i=0; i<sTypes.size(); i++) {
			if(sTypes[i]->name == name) {
				return sTypes[i];
			}
		}

		return 0;
	}
}