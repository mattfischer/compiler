#ifndef OPT_PASS_INTO_SSA_H
#define OPT_PASS_INTO_SSA_H

#include "Optimizer.h"

class OptPassIntoSSA : public Optimizer::Pass {
public:
	OptPassIntoSSA() : Pass("IntoSSA") {}

	virtual bool procedures() { return true; }

	virtual bool optimizeProcedure(IR::Procedure *proc);
};
#endif