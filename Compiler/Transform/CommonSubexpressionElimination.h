#ifndef TRANSFORM_COMMON_SUBEXPRESSION_ELIMINATION_H
#define TRANSFORM_COMMON_SUBEXPRESSION_ELIMINATION_H

#include "Transform/Transform.h"

namespace Transform {
	class CommonSubexpressionElimination : public Transform {
	public:
		virtual bool transform(IR::Procedure *procedure, Analysis::Analysis &analysis);
		virtual std::string name() { return "CommonSubexpressionElimination"; }

		static CommonSubexpressionElimination *instance();
	};
}
#endif