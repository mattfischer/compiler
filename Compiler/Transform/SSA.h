#ifndef TRANSFORM_SSA_H
#define TRANSFORM_SSA_H

#include <string>

#include "Transform/Transform.h"

namespace IR {
	class Symbol;
}

namespace Transform {
	class SSA : public Transform {
	public:
		virtual bool transform(IR::Procedure *procedure, Analysis::Analysis &analysis);
		virtual std::string name() { return "SSA"; }

		static SSA *instance();

	private:
		std::string newSymbolName(IR::Symbol *base, int version);
	};
}
#endif