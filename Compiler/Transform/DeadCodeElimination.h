#ifndef TRANSFORM_DEAD_CODE_ELIMINATION_H
#define TRANSFORM_DEAD_CODE_ELIMINATION_H

#include "Transform/Transform.h"

namespace Transform {
	/*!
	 * \brief Perform dead code elimination on a procedure
	 *
	 * Dead code is defined as any instruction not necessary for the correct functioning
	 * of the procedure.  This includes assignments to variables which are never read, as
	 * well as blocks of code which cannot be reached by any path through the control flow graph.
	 */
	class DeadCodeElimination : public Transform {
	public:
		virtual bool transform(IR::Procedure *procedure);
		virtual std::string name() { return "DeadCodeElimination"; }

		static DeadCodeElimination *instance();
	};
}
#endif