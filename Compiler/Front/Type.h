#ifndef	TYPE_H
#define TYPE_H

#include <string>
#include <vector>

namespace Front {
	class Scope;

	/*!
	 * \brief Represents a type in the type system
	 */
	class Type {
	public:
		enum TypeType {
			TypeIntrinsic,
			TypeProcedure,
			TypeArray,
			TypeStruct,
			TypeClass
		};

		Type(TypeType _type, std::string _name, int _valueSize, int _allocSize)
			: type(_type),
			  name(_name),
			  valueSize(_valueSize),
			  allocSize(_allocSize)
		{
		}

		TypeType type;
		std::string name;
		int valueSize;
		int allocSize;

		static bool equals(Type *a, Type *b);

	private:
		static std::vector<Type*> sTypes;
	};

	/*!
	 * \brief An intrinsic type
	 */
	class TypeIntrinsic : public Type {
	public:
		TypeIntrinsic(std::string _name, int _size)
			: Type(Type::TypeIntrinsic, _name, _size, _size)
		{
		}
	};

	/*!
	 * \brief A procedure type
	 */
	class TypeProcedure : public Type {
	public:
		Type *returnType;
		std::vector<Type*> argumentTypes;

		TypeProcedure(Type *_returnType, std::vector<Type*> _argumentTypes)
			: Type(Type::TypeProcedure, getTypeName(_returnType, _argumentTypes), 0, 0), returnType(_returnType), argumentTypes(_argumentTypes)
		{}

	private:
		std::string getTypeName(Type *returnType, std::vector<Type*> argumentTypes);
	};

	/*!
	 * \brief An array type
	 */
	class TypeArray : public Type {
	public:
		Type *baseType;

		TypeArray(Type *_baseType)
			: Type(Type::TypeArray, _baseType->name + "[]", 4, 0), baseType(_baseType)
		{}
	};

	/*!
	 * \brief A struct type
	 */
	class TypeStruct : public Type {
	public:
		struct Member {
			Type *type;
			std::string name;
			bool virtualFunction;
			int offset;
		};

		std::vector<Member> members;
		Front::TypeProcedure *constructor;
		Scope *scope;
		TypeStruct *parent;
		int vtableSize;
		int vtableOffset;

		TypeStruct(TypeType _type, const std::string &_name)
			: Type(_type, _name, 4, 0)
		{}

		void addMember(Type *type, const std::string &name, bool virtualFunction);
		Member *findMember(const std::string &name);
	};
}

#endif