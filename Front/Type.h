#ifndef	TYPE_H
#define TYPE_H

#include <string>
#include <vector>
#include <memory>

namespace Front {
	class Scope;

	/*!
	 * \brief Represents a type in the type system
	 */
	class Type {
	public:
		enum class Kind {
			Intrinsic,
			Procedure,
			Array,
			Struct,
			Class,
			Dummy
		};

		Type(Kind _kind, std::string _name, int _valueSize, int _allocSize)
			: kind(_kind),
			  name(_name),
			  valueSize(_valueSize),
			  allocSize(_allocSize)
		{
		}

		Kind kind;
		std::string name;
		int valueSize;
		int allocSize;

		static bool equals(Type &a, Type &b);
	};

	/*!
	 * \brief An intrinsic type
	 */
	class TypeIntrinsic : public Type {
	public:
		TypeIntrinsic(std::string _name, int _size)
			: Type(Kind::Intrinsic, _name, _size, _size)
		{
		}
	};

	/*!
	 * \brief A procedure type
	 */
	class TypeProcedure : public Type {
	public:
		std::shared_ptr<Type> returnType;
		std::vector<std::shared_ptr<Type>> argumentTypes;

		TypeProcedure(std::shared_ptr<Type> &_returnType, std::vector<std::shared_ptr<Type>> _argumentTypes)
			: Type(Kind::Procedure, getTypeName(_returnType, _argumentTypes), 0, 0), returnType(_returnType), argumentTypes(_argumentTypes)
		{}

	private:
		std::string getTypeName(std::shared_ptr<Type> &returnType, std::vector<std::shared_ptr<Type>> &argumentTypes);
	};

	/*!
	 * \brief An array type
	 */
	class TypeArray : public Type {
	public:
		std::shared_ptr<Type> baseType;

		TypeArray(std::shared_ptr<Type> &_baseType)
			: Type(Kind::Array, _baseType->name + "[]", 4, 0), baseType(_baseType)
		{}
	};

	/*!
	 * \brief A struct type
	 */
	class TypeStruct : public Type {
	public:
		struct Member {
			enum Qualifier {
				QualifierVirtual = 0x1,
				QualifierNative = 0x2,
				QualifierStatic = 0x4
			};
			std::shared_ptr<Type> type;
			std::string name;
			unsigned int qualifiers;
			int offset;
		};

		std::vector<Member> members;
		std::shared_ptr<Front::TypeProcedure> constructor;
		Scope *scope;
		std::shared_ptr<TypeStruct> parent;
		int vtableSize;
		int vtableOffset;

		TypeStruct(Kind _kind, const std::string &_name)
			: Type(_kind, _name, 4, 0)
		{}

		void addMember(std::shared_ptr<Type> type, const std::string &name, unsigned int qualifiers);
		Member *findMember(const std::string &name);
	};

	/*!
	 * \brief A placeholder type
	 */
	class TypeDummy : public Type {
	public:
		std::string origin;

		TypeDummy(const std::string &_name, const std::string &_origin)
			: Type(Kind::Dummy, _name, 0, 0), origin(_origin)
		{}
	};
}

#endif