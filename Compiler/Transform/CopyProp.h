#ifndef TRANSFORM_COPY_PROP_H
#define TRANSFORM_COPY_PROP_H

#include "Transform/Transform.h"

namespace Transform {
	/*!
	 * \brief Propagate copies through a procedure
	 *
	 * Copy propagation attempts to locate copies of one symbol into another.  In
	 * cases where the symbol is only assigned from this location, the original symbol
	 * can simply be substituted in its place, and the copy entry eliminated completely.
	 */
	class CopyProp : public Transform {
	public:
		virtual bool transform(IR::Procedure *procedure, Analysis::Analysis &analysis);
		virtual std::string name() { return "CopyProp"; }

		static CopyProp *instance();
	private:
		bool forward(IR::Procedure *procedure, Analysis::Analysis &analysis);
		bool backward(IR::Procedure *procedure, Analysis::Analysis &analysis);
	};
}
#endif