#ifndef TRANSFORM_SSA_H
#define TRANSFORM_SSA_H

#include <string>

#include "Analysis/Analysis.h"

namespace IR {
	class Procedure;
	class Symbol;
}

namespace Transform {
	class SSA {
	public:
		static void transform(IR::Procedure *procedure, Analysis::Analysis &analysis);

	private:
		static std::string newSymbolName(IR::Symbol *base, int version);
	};
}
#endif