#ifndef TRANSFORM_CONSTANT_PROP_H
#define TRANSFORM_CONSTANT_PROP_H

namespace IR {
	class Procedure;
	class Entry;
	class Symbol;
}

namespace Analysis {
	class UseDefs;
}

namespace Transform {
	class ConstantProp {
	public:
		static void transform(IR::Procedure *procedure, Analysis::UseDefs &useDefs);

	private:
		static int getValue(IR::Entry *entry, IR::Symbol *symbol, const Analysis::UseDefs &useDefs, bool &isConstant);
	};
}
#endif
