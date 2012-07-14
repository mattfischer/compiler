#ifndef MIDDLE_OPTIMIZER_H
#define MIDDLE_OPTIMIZER_H

#include "IR/Program.h"

namespace Middle {
	class Optimizer {
	public:
		static void optimize(IR::Program *program);
	};
}
#endif