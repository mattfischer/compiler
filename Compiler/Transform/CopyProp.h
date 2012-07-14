#ifndef TRANSFORM_COPY_PROP_H
#define TRANSFORM_COPY_PROP_H

#include "Transform/Transform.h"

namespace Transform {
	class CopyProp : public Transform {
	public:
		void transform(IR::Procedure *procedure, Analysis::Analysis &analysis);
		std::string name() { return "CopyProp"; }

		static CopyProp *instance();
	};
}
#endif