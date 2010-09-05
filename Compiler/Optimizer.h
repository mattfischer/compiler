#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "IR.h"

#include <string>
#include <vector>

class Optimizer {
public:
	class Pass {
	public:
		Pass(const std::string &name) : mName(name) {}

		const std::string &name() const { return mName; }

		virtual bool procedures() { return false; }

		virtual void optimizeProcedure(IR::Procedure *proc) {}

	private:
		std::string mName;
	};

	Optimizer(IR *ir);

	void optimize();

	IR *ir() { return mIR; }

private:
	IR *mIR;
	std::vector<Pass*> mPasses;

	void initPasses();
};

#endif