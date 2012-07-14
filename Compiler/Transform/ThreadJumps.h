#ifndef TRANSFORM_THREAD_JUMPS_H
#define TRANSFORM_THREAD_JUMPS_H

#include "Transform/Transform.h"

namespace IR {
	struct EntryLabel;
}

namespace Transform {
	class ThreadJumps : public Transform {
	public:
		void transform(IR::Procedure *procedure, Analysis::Analysis &analysis);
		std::string name() { return "ThreadJumps"; }

		ThreadJumps *instance();

	private:
		IR::EntryLabel *getJumpTarget(IR::EntryLabel *label);
	};
}

#endif