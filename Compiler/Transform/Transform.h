#ifndef TRANSFORM_TRANSFORM_H
#define TRANSFORM_TRANSFORM_H

#include <string>

#include "IR/Procedure.h"

namespace Transform {
	class Transform {
	public:
		virtual bool transform(IR::Procedure *procedure) = 0;
		virtual std::string name() = 0;
	};
}
#endif