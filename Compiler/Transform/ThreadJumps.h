#ifndef TRANSFORM_THREAD_JUMPS_H
#define TRANSFORM_THREAD_JUMPS_H

namespace IR {
	class Procedure;
	struct EntryLabel;
}

namespace Transform {
	class ThreadJumps {
	public:
		static void transform(IR::Procedure *procedure);

	private:
		static IR::EntryLabel *getJumpTarget(IR::EntryLabel *label);
	};
}

#endif