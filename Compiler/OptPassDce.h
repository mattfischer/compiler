#ifndef OPT_PASS_DCE_H
#define OPT_PASS_DCE_H

#include "Optimizer.h"

class OptPassDce : public Optimizer::Pass {
public:
	OptPassDce() : Pass("Dce") {}

	virtual void optimize(IR *ir);
};

#endif