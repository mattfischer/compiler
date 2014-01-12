#ifndef TRANSFORM_LOOP_INVARIANT_CODE_MOTION_H
#define TRANSFORM_LOOP_INVARIANT_CODE_MOTION_H

#include "Transform/Transform.h"

#include "Analysis/Loops.h"

namespace Transform {
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