#ifndef TRANSFORM_CONSTANT_PROP_H
#define TRANSFORM_CONSTANT_PROP_H

#include "Transform/Transform.h"

#include "IR/Entry.h"
#include "IR/Symbol.h"

#include "Analysis/UseDefs.h"

namespace Transform {
	class ConstantProp : public Transform {
	public:
		virtual bool transform(IR::Procedure *procedure);
		virtual std::string name() { return "ConstantProp"; }

		static ConstantProp *instance();

	private:
		int getValue(IR::Entry *entry, IR::Symbol *symbol, Analysis::UseDefs &useDefs, bool &isConstant);
	};
}
#endif
