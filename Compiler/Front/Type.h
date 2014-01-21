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
			: Type(Type::TypeProcedure, "", 0), returnType(_returnType), argumentTypes(_argumentTypes)
		{}
	};

	extern Type *TypeNone;
	extern Type *TypeInt;
	extern Type *TypeBool;
}

#endif