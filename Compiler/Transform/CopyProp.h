#ifndef TRANSFORM_COPY_PROP_H
#define TRANSFORM_COPY_PROP_H

namespace IR {
	class Procedure;
}

namespace Analysis {
	class UseDefs;
	class ReachingDefs;
}

namespace Transform {
	class CopyProp {
	public:
		static void transform(IR::Procedure *procedure, Analysis::UseDefs &useDefs, Analysis::ReachingDefs &reachingDefs);
	};
}
#endif