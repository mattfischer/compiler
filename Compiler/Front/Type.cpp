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

	bool Type::equals(Type *a, Type *b)
	{
		if(a->type != b->type) {
			return false;
		}

		switch(a->type) {
			case TypeIntrinsic:
				return ((Front::TypeIntrinsic*)a)->intrinsicType == ((Front::TypeIntrinsic*)b)->intrinsicType;

			case TypeProcedure:
			{
				Front::TypeProcedure *ap = (Front::TypeProcedure*)a;
				Front::TypeProcedure *bp = (Front::TypeProcedure*)b;
				if(!equals(ap->returnType, bp->returnType)) {
					return false;
				}

				if(ap->argumentTypes.size() != bp->argumentTypes.size()) {
					return false;
				}

				for(unsigned int i=0; i<ap->argumentTypes.size(); i++) {
					if(!equals(ap->argumentTypes[i], bp->argumentTypes[i])) {
						return false;
					}
				}

				return true;
			}

			case TypeNone:
				return true;
		}

		return false;
	}
}