#ifndef TRANSFORM_DEAD_CODE_ELIMINATION_H
#define TRANSFORM_DEAD_CODE_ELIMINATION_H

#include "Analysis/Analysis.h"

namespace IR {
	class Procedure;
}

namespace Transform {
	class DeadCodeElimination {
	public:
		static void transform(IR::Procedure *procedure, Analysis::Analysis &analysis);
	};
}
#endif