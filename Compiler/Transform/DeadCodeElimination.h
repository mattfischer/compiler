#ifndef TRANSFORM_DEAD_CODE_ELIMINATION_H
#define TRANSFORM_DEAD_CODE_ELIMINATION_H

namespace IR {
	class Procedure;
}

namespace Analysis {
	class UseDefs;
	class FlowGraph;
	class ReachingDefs;
}

namespace Transform {
	class DeadCodeElimination {
	public:
		static void transform(IR::Procedure *procedure, Analysis::UseDefs &useDefs, Analysis::ReachingDefs &reachingDefs, Analysis::FlowGraph &flowGraph);
	};
}
#endif