#ifndef	TYPE_H
#define TYPE_H

#include <string>
#include <vector>

struct Type {
	enum TypeType {
		TypeIntrinsic,
		TypeNone
	};

	Type(TypeType _type, std::string _name)
		: type(_type),
		  name(_name)
	{
	}

	TypeType type;
	std::string name;

	static void init();
	static Type *find(const std::string& name);

private:
	static std::vector<Type*> sTypes;
};

struct TypeIntrinsic : public Type {
	enum IntrinsicType {
		Int
	};

	TypeIntrinsic(IntrinsicType _intrinsicType, std::string _name)
		: Type(Type::TypeIntrinsic, _name),
		  intrinsicType(_intrinsicType)
	{
	}

	IntrinsicType intrinsicType;
};

extern Type *TypeNone;
extern Type *TypeInt;

#endif