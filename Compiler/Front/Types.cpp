#include "Front/Types.h"

namespace Front {
	/*!
	 * \brief Constructor
	 */
	Types::Types()
	{
		// Add all intrinsic types into the type list
		for(int i=0; i<NumIntrinsics; i++) {
			mTypes.push_back(intrinsic((Intrinsic)i));
		}
	}

	Types::~Types()
	{
	}

	/*!
	 * \brief Register a new type with the type system
	 * \param type Type to register
	 * \return True if success
	 */
	bool Types::registerType(Type *type)
	{
		if(findType(type->name)) {
			return false;
		}

		mTypes.push_back(type);

		return true;
	}

	/*!
	 * \brief Find the type with a given name
	 * \param name Type name
	 * \return Type if found, or 0
	 */
	Type *Types::findType(const std::string &name)
	{
		for(Type *type : mTypes) {
			if(type->name == name) {
				return type;
			}
		}

		return 0;
	}

	/*!
	 * \brief Retrieve an intrinsic type
	 * \param intrinsic Intrinsic type number
	 * \return Intrinsic type
	 */
	Type *Types::intrinsic(Intrinsic intrinsic)
	{
		static std::vector<Type*> intrinsics;

		if(intrinsics.size() == 0) {
			// Register all the intrinsics on first access
			intrinsics.resize(NumIntrinsics);
			intrinsics[Bool] = new TypeIntrinsic("bool", 4);
			intrinsics[Int] = new TypeIntrinsic("int", 4);
			intrinsics[Void] = new TypeIntrinsic("void", 0);
			intrinsics[Char] = new TypeIntrinsic("char", 1);
			intrinsics[String] = new TypeIntrinsic("string", 4);
		}

		return intrinsics[intrinsic];
	}
}