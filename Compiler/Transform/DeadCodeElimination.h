#ifndef TRANSFORM_DEAD_CODE_ELIMINATION_H
#define TRANSFORM_DEAD_CODE_ELIMINATION_H

#include "Transform/Transform.h"

namespace Transform {
	class DeadCodeElimination : public Transform {
	public:
		virtual bool transform(IR::Procedure *procedure);
		virtual std::string name() { return "DeadCodeElimination"; }

		static DeadCodeElimination *instance();
	};
}
#endif