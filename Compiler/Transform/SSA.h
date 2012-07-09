#ifndef TRANSFORM_SSA_H
#define TRANSFORM_SSA_H

#include <string>

namespace IR {
	class Procedure;
	class Symbol;
}

namespace Transform {
	class SSA {
	public:
		static void transform(IR::Procedure *procedure);

	private:
		static std::string newSymbolName(IR::Symbol *base, int version);
	};
}
#endif