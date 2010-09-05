#include "Optimizer.h"

#include "OptPassJumps.h"
#include "OptPassDce.h"

Optimizer::Optimizer(IR *ir)
{
	mIR = ir;

	initPasses();
}

void Optimizer::optimize()
{
	printf("BEFORE OPTIMIZATION:\n");
	mIR->print();
	printf("\n");

	for(unsigned int i=0; i<mPasses.size(); i++) {
		if(mPasses[i]->procedures()) {
			for(int j=0; j<mIR->procedures().size(); j++) {
				mPasses[i]->optimizeProcedure(mIR->procedures()[j]);
			}
		}

		printf("PASS \"%s\":\n", mPasses[i]->name().c_str());
		mIR->print();
		printf("\n");
	}
}

void Optimizer::initPasses()
{
	mPasses.push_back(new OptPassJumps);
	mPasses.push_back(new OptPassDce);
}