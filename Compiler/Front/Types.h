#ifndef FRONT_TYPES_H
#define FRONT_TYPES_H

#include "Front/Type.h"

#include <vector>
#include <memory>

namespace Front {
	/*!
	 * \brief Represents a collection of types
	 */
	class Types {
	public:
		Types();
		~Types();

		bool registerType(std::unique_ptr<Type> type);
		Type *findType(const std::string &name);

		const std::vector<Type*> &types() { return mTypes; }

		enum Intrinsic {
			Bool,
			Int,
			Void,
			String,
			Char,
			NumIntrinsics
		};
		static Type *intrinsic(Intrinsic intrinsic);

	private:
		std::vector<Type*> mTypes;
		std::vector<std::unique_ptr<Type>> mOwnedTypes;
	};
}
#endif