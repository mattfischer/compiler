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

		bool registerType(std::shared_ptr<Type> type);
		std::shared_ptr<Type> findType(const std::string &name);

		std::vector<std::shared_ptr<Type>> &types() { return mTypes; }

		enum Intrinsic {
			Bool,
			Int,
			Void,
			String,
			Char,
			NumIntrinsics
		};
		static std::shared_ptr<Type> &intrinsic(Intrinsic intrinsic);

	private:
		std::vector<std::shared_ptr<Type>> mTypes;
	};
}
#endif