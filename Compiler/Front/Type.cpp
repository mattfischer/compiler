#include "Type.h"

#include <sstream>

namespace Front {
	bool Type::equals(Type *a, Type *b)
	{
		if(a->type != b->type) {
			return false;
		}

		switch(a->type) {
			case TypeIntrinsic:
			case TypeStruct:
				return a == b;

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

			case TypeArray:
				return Type::equals(((Front::TypeArray*)a)->baseType, ((Front::TypeArray*)b)->baseType);
		}

		return false;
	}

	std::string TypeProcedure::getTypeName(Type *returnType, std::vector<Type*> argumentTypes)
	{
		std::stringstream s;
		s << returnType->name << "(";
		for(unsigned int i=0; i<argumentTypes.size(); i++) {
			s << argumentTypes[i]->name;
			if(i < argumentTypes.size() - 1) {
				s << ", ";
			}
		}
		s << ")";
		return s.str();
	}

	void TypeStruct::addMember(Type *type, const std::string &name)
	{
		Member member;
		member.name = name;
		member.type = type;
		member.offset = size;
		members.push_back(member);

		size += type->size;
	}
}