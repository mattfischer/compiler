#ifndef FRONT_TYPES_H
#define FRONT_TYPES_H

#include "Front/Type.h"

#include <vector>

namespace Front {
	class Types {
	public:
		Types();
		~Types();

		bool registerType(Type *type);
		Type *findType(const std::string &name);

		enum Intrinsic {
			Bool,
			Int,
			Void,
			String,
			NumIntrinsics
		};
		static Type *intrinsic(Intrinsic intrinsic);

	private:
		std::vector<Type*> mTypes;
	};
}
#endif