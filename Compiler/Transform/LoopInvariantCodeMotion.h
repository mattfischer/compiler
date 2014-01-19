#ifndef TRANSFORM_LOOP_INVARIANT_CODE_MOTION_H
#define TRANSFORM_LOOP_INVARIANT_CODE_MOTION_H

#include "Transform/Transform.h"

#include "Analysis/Loops.h"

namespace Transform {
	/*!
	 * \brief Perform loop-invariant code motion on a procedure
	 *
	 * Any assignment which takes place inside of a loop, but whose arguments have the same value
	 * in each iteration of the loop, can be moved out of the loop to improve performance
	 */
	class LoopInvariantCodeMotion : public Transform {
	public:
		virtual bool transform(IR::Procedure *procedure);
		virtual std::string name() { return "LoopInvariantCodeMotion"; }

		static LoopInvariantCodeMotion *instance();

	private:
		void processLoop(Analysis::Loops::Loop *loop, IR::Procedure *procedure, Analysis::Loops &loops);
	};
}
#endif