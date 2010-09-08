#ifndef OPT_PASS_DCE_H
#define OPT_PASS_DCE_H

#include "Optimizer.h"

class OptPassDce : public Optimizer::Pass {
public:
	OptPassDce() : Pass("DCE") {}

	virtual bool procedures() { return true; }

	virtual void optimizeProcedure(IR::Procedure *proc);
};

#endif