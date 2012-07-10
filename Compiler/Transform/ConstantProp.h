#ifndef TRANSFORM_CONSTANT_PROP_H
#define TRANSFORM_CONSTANT_PROP_H

namespace IR {
	class Procedure;
	class Entry;
	class Symbol;
}

namespace Analysis {
	class UseDefs;
	class ReachingDefs;
	class FlowGraph;
}

namespace Transform {
	class ConstantProp {
	public:
		static void transform(IR::Procedure *procedure, Analysis::UseDefs &useDefs, Analysis::ReachingDefs &reachingDefs, Analysis::FlowGraph &flowGraph);

	private:
		static int getValue(IR::Entry *entry, IR::Symbol *symbol, const Analysis::UseDefs &useDefs, bool &isConstant);
	};
}
#endif
