#ifndef	TYPE_H
#define TYPE_H

#include <string>
#include <vector>

namespace Front {
	struct Type {
		enum TypeType {
			TypeIntrinsic,
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

	struct TypeIntrinsic : public Type {
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

	extern Type *TypeNone;
	extern Type *TypeInt;
	extern Type *TypeBool;
}

#endif