#ifndef TRANSFORM_COPY_PROP_H
#define TRANSFORM_COPY_PROP_H

#include "Analysis/Analysis.h"

namespace IR {
	class Procedure;
}

namespace Transform {
	class CopyProp {
	public:
		static void transform(IR::Procedure *procedure, Analysis::Analysis &analysis);
	};
}
#endif