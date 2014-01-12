#ifndef TRANSFORM_SSA_H
#define TRANSFORM_SSA_H

#include "Transform/Transform.h"

#include "IR/Symbol.h"

#include <string>

namespace Transform {
	class SSA : public Transform {
	public:
		virtual bool transform(IR::Procedure *procedure);
		virtual std::string name() { return "SSA"; }

		static SSA *instance();

	private:
		std::string newSymbolName(IR::Symbol *base, int version);
	};
}
#endif