#ifndef TRANSFORM_COPY_PROP_H
#define TRANSFORM_COPY_PROP_H

#include "Transform/Transform.h"

namespace Transform {
	class CopyProp : public Transform {
	public:
		virtual bool transform(IR::Procedure *procedure);
		virtual std::string name() { return "CopyProp"; }

		static CopyProp *instance();
	private:
		bool forward(IR::Procedure *procedure);
		bool backward(IR::Procedure *procedure);
	};
}
#endif