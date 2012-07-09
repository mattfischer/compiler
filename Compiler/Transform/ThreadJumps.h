#ifndef TRANSFORM_THREAD_JUMPS_H
#define TRANSFORM_THREAD_JUMPS_H

namespace IR {
	class Procedure;
	class Block;
}

namespace Transform {
	class ThreadJumps {
	public:
		static void transform(IR::Procedure *procedure);

	private:
		static IR::Block *getJumpTarget(IR::Block *block);
	};
}

#endif