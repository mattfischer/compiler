#ifndef OPT_PASS_COPY_H
#define OPT_PASS_COPY_H

#include "Optimizer.h"

class OptPassCopy : public Optimizer::Pass
{
public:
	OptPassCopy() : Pass("Copy") {}

	virtual bool procedures() { return true; }

	virtual void optimizeProcedure(IR::Procedure *proc);
};

#endif