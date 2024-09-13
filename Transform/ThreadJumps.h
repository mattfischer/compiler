#ifndef TRANSFORM_THREAD_JUMPS_H
#define TRANSFORM_THREAD_JUMPS_H

#include "Transform/Transform.h"

#include "IR/Entry.h"

namespace Transform {
	/*!
	 * \brief Perform jump threading on a procedure
	 *
	 * Jump threading looks for jumps whose target is another jump, and updates
	 * their targets to their final location to bypass the unnecessary jumps
	 */
	class ThreadJumps : public Transform {
	public:
		virtual bool transform(IR::Procedure &procedure, Analysis::Analysis &analysis);
		virtual std::string name() { return "ThreadJumps"; }

		static ThreadJumps *instance();

	private:
		IR::EntryLabel *getJumpTarget(IR::EntryLabel *label);
	};
}

#endif