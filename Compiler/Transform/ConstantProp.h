#ifndef TRANSFORM_CONSTANT_PROP_H
#define TRANSFORM_CONSTANT_PROP_H

#include "Transform/Transform.h"

namespace IR {
	class Entry;
	class Symbol;
}

namespace Analysis {
	class UseDefs;
}

namespace Transform {
	class ConstantProp : public Transform {
	public:
		void transform(IR::Procedure *procedure, Analysis::Analysis &analysis);

		static ConstantProp &instance();

	private:
		int getValue(IR::Entry *entry, IR::Symbol *symbol, Analysis::UseDefs &useDefs, bool &isConstant);
	};
}
#endif
