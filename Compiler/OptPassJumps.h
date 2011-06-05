#ifndef OPT_PASS_JUMPS_H
#define OPT_PASS_JUMPS_H

#include "Optimizer.h"

class OptPassJumps : public Optimizer::Pass {
public:
	OptPassJumps() : Pass("Jumps") {}

	virtual bool procedures() { return true; }

	virtual bool optimizeProcedure(IR::Procedure *proc);
};

#endif