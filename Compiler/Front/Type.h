#ifndef	TYPE_H
#define TYPE_H

#include <string>
#include <vector>

namespace Front {
	class Type {
	public:
		enum TypeType {
			TypeIntrinsic,
			TypeProcedure,
			TypeArray,
			TypeStruct
		};

		Type(TypeType _type, std::string _name, int _size)
			: type(_type),
			  name(_name),
			  size(_size)
		{
		}

		TypeType type;
		std::string name;
		int size;

		static bool equals(Type *a, Type *b);

	private:
		static std::vector<Type*> sTypes;
	};

	class TypeIntrinsic : public Type {
	public:
		TypeIntrinsic(std::string _name, int _size)
			: Type(Type::TypeIntrinsic, _name, _size)
		{
		}
	};

	class TypeProcedure : public Type {
	public:
		Type *returnType;
		std::vector<Type*> argumentTypes;

		TypeProcedure(Type *_returnType, std::vector<Type*> _argumentTypes)
			: Type(Type::TypeProcedure, getTypeName(_returnType, _argumentTypes), 0), returnType(_returnType), argumentTypes(_argumentTypes)
		{}

	private:
		std::string getTypeName(Type *returnType, std::vector<Type*> argumentTypes);
	};

	class TypeArray : public Type {
	public:
		Type *baseType;

		TypeArray(Type *_baseType)
			: Type(Type::TypeArray, _baseType->name + "[]", 0), baseType(_baseType)
		{}
	};

	class TypeStruct : public Type {
	public:
		struct Member {
			Type *type;
			std::string name;
			int offset;
		};

		std::vector<Member> members;

		TypeStruct(const std::string &_name)
			: Type(Type::TypeStruct, _name, 0)
		{}

		void addMember(Type *type, const std::string &name);
	};
}

#endif