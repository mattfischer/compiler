#include "Front/Types.h"

namespace Front {
	Types::Types()
	{
		for(int i=0; i<NumIntrinsics; i++) {
			mTypes.push_back(intrinsic((Intrinsic)i));
		}
	}

	bool Types::registerType(Type *type)
	{
		if(findType(type->name)) {
			return false;
		}

		mTypes.push_back(type);
		return true;
	}

	Type *Types::findType(const std::string &name)
	{
		for(unsigned int i=0; i<mTypes.size(); i++) {
			if(mTypes[i]->name == name) {
				return mTypes[i];
			}
		}

		return 0;
	}

	Type *Types::intrinsic(Intrinsic intrinsic)
	{
		static std::vector<Type*> intrinsics;

		if(intrinsics.size() == 0) {
			intrinsics.resize(NumIntrinsics);
			intrinsics[Bool] = new TypeIntrinsic("bool", 4);
			intrinsics[Int] = new TypeIntrinsic("int", 8);
			intrinsics[Void] = new TypeIntrinsic("void", 0);
		}

		return intrinsics[intrinsic];
	}
}