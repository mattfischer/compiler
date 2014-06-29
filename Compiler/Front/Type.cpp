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
		if(a->type != b->type) {
			return false;
		}

		switch(a->type) {
			case TypeIntrinsic:
			case TypeStruct:
			case TypeClass:
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
	void TypeStruct::addMember(Type *type, const std::string &name, bool virtualFunction)
	{
		Member member;
		member.name = name;
		member.type = type;
		member.virtualFunction = virtualFunction;

		if(type->type == Type::TypeProcedure) {
			if(virtualFunction) {
				bool overridden = false;
				if(parent) {
					Member *parentMember = parent->findMember(name);
					if(parentMember) {
						member.offset = parentMember->offset;
						overridden = true;
					}
				}

				if(!overridden) {
					if(vtableSize == 0) {
						vtableOffset = allocSize;
						allocSize += 4;
					}
					member.offset = vtableSize;
					vtableSize++;
				}
			} else {
				member.offset = -1;
			}
		} else {
			member.offset = allocSize;
			allocSize += type->valueSize;
		}

		members.push_back(member);
	}

	TypeStruct::Member *TypeStruct::findMember(const std::string &name)
	{
		for(unsigned int i=0; i<members.size(); i++) {
			if(members[i].name == name) {
				return &members[i];
			}
		}

		if(parent) {
			return parent->findMember(name);
		} else {
			return 0;
		}
	}
}