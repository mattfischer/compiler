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
			TypeNone
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

		static void init();
		static Type *find(const std::string& name);
		static bool equals(Type *a, Type *b);

	private:
		static std::vector<Type*> sTypes;
	};

	class TypeIntrinsic : public Type {
	public:
		enum IntrinsicType {
			Int,
			Bool
		};

		TypeIntrinsic(IntrinsicType _intrinsicType, std::string _name, int _size)
			: Type(Type::TypeIntrinsic, _name, _size),
			  intrinsicType(_intrinsicType)
		{
		}

		IntrinsicType intrinsicType;
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

	extern Type *TypeNone;
	extern Type *TypeInt;
	extern Type *TypeBool;
}

#endif