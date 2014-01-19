#ifndef TRANSFORM_CONSTANT_PROP_H
#define TRANSFORM_CONSTANT_PROP_H

#include "Transform/Transform.h"

#include "IR/Entry.h"
#include "IR/Symbol.h"

#include "Analysis/UseDefs.h"

namespace Transform {
	/*!
	 * \brief Propagate constants through a procedure
	 *
	 * This transform examines use-def information to determine when an entry can be
	 * guaranteed to evaluate to a constant value.  In cases where this is possible, it
	 * substitutes Load Immediate entries into the procedure in place of the original
	 * arithmetic instruction
	 */
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
