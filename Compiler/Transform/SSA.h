#ifndef TRANSFORM_SSA_H
#define TRANSFORM_SSA_H

#include "Transform/Transform.h"

#include "IR/Symbol.h"

#include <string>

namespace Transform {
	/*!
	 * \brief Transform the procedure into Static Single-Assignment form (SSA)
	 *
	 * In SSA form, each symbol is assigned to exactly once.
	 */
	class SSA : public Transform {
	public:
		virtual bool transform(IR::Procedure *procedure);
		virtual std::string name() { return "SSA"; }

		static SSA *instance();

	private:
		std::string newSymbolName(IR::Symbol *base, int version);
	};
}
#endif