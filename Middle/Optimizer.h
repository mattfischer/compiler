#ifndef MIDDLE_OPTIMIZER_H
#define MIDDLE_OPTIMIZER_H

#include "IR/Program.h"

/*!
 * \brief Classes which operate on the IR before it is turned into VM code
 */
namespace Middle {
	/*!
	 * \brief Optimize a program
	 *
	 * Top-level driver for all optimization passes
	 */
	class Optimizer {
	public:
		static void optimize(IR::Program &program);
	};
}
#endif