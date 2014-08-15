#include "Type.h"

#include <sstream>

namespace Front {
	/*!
	 * \brief Check two types for equality
	 * \param a First type
	 * \param b Second type
	 * \return True if types are equal
	 */
	bool Type::equals(Type *a, Type *b)
	{
		if(a->kind != b->kind) {
			return false;
		}

		switch(a->kind) {
			case Type::Kind::Intrinsic:
			case Type::Kind::Struct:
			case Type::Kind::Class:
			case Type::Kind::Dummy:
				return a == b;

			case Type::Kind::Procedure:
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

			case Type::Kind::Array:
				return Type::equals(((Front::TypeArray*)a)->baseType, ((Front::TypeArray*)b)->baseType);
		}

		return false;
	}

	/*!
	 * \brief Get type name for a procedure type
	 * \param returnType Return type
	 * \param argumentTypes Argument types
	 * \return Type name
	 */
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

	/*!
	 * \brief Add a member to a struct type
	 * \param type Type of member
	 * \param name Member name
	 */
	void TypeStruct::addMember(Type *type, const std::string &name, unsigned int qualifiers)
	{
		Member member;
		member.name = name;
		member.type = type;
		member.qualifiers = qualifiers;

		members.push_back(member);
	}

	TypeStruct::Member *TypeStruct::findMember(const std::string &name)
	{
		for(Member &member : members) {
			if(member.name == name) {
				return &member;
			}
		}

		if(parent) {
			return parent->findMember(name);
		} else {
			return 0;
		}
	}
}