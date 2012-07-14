#ifndef TRANSFORM_DEAD_CODE_ELIMINATION_H
#define TRANSFORM_DEAD_CODE_ELIMINATION_H

#include "Transform/Transform.h"

namespace Transform {
	class DeadCodeElimination : public Transform {
	public:
		void transform(IR::Procedure *procedure, Analysis::Analysis &analysis);

		static DeadCodeElimination &instance();
	};
}
#endif