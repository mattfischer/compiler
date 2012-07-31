#ifndef TRANSFORM_TRANSFORM_H
#define TRANSFORM_TRANSFORM_H

#include <string>

namespace IR {
	class Procedure;
}

namespace Transform {
	class Transform {
	public:
		virtual bool transform(IR::Procedure *procedure) = 0;
		virtual std::string name() = 0;
	};
}
#endif