#ifndef OPT_PASS_CONSTANT_H
#define OPT_PASS_CONSTANT_H

#include "Optimizer.h"

class OptPassConstant : public Optimizer::Pass
{
public:
	OptPassConstant() : Pass("Constant") {}

	virtual bool procedures() { return true; }

	virtual void optimizeProcedure(IR::Procedure *proc);
};
#endif