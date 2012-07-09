#include "Optimizer.h"

#include "IR/Program.h"

#include "OptPassJumps.h"
#include "OptPassDce.h"
#include "OptPassIntoSSA.h"
#include "OptPassCopy.h"

Optimizer::Optimizer(IR::Program *ir)
{
	mIR = ir;

	initPasses();
}

void Optimizer::optimize()
{
	printf("BEFORE OPTIMIZATION:\n");
	mIR->print();
	printf("\n");

	repeatPasses(mPreSSAPasses);

	optimizeProcedures(new OptPassIntoSSA);
	printf("INTO SSA:\n");
	mIR->print();
	printf("\n");

	repeatPasses(mPostSSAPasses);
}

void Optimizer::repeatPasses(std::vector<Pass*> &passes)
{
	bool changed;
	do {
		changed = false;
		
		for(unsigned int i=0; i<passes.size(); i++) {
			if(optimizeProcedures(passes[i])) {
				changed = true;
			}

			printf("PASS \"%s\":\n", passes[i]->name().c_str());
			mIR->print();
			printf("\n");
		}
	} while(changed);
}

bool Optimizer::optimizeProcedures(Pass *pass)
{
	bool changed = false;
	for(unsigned int j=0; j<mIR->procedures().size(); j++) {
		while(pass->optimizeProcedure(mIR->procedures()[j])) {
			changed = true;
		}
	}

	return changed;
}

void Optimizer::initPasses()
{
	mPreSSAPasses.push_back(new OptPassJumps);
	mPreSSAPasses.push_back(new OptPassDce);

	mPostSSAPasses.push_back(new OptPassCopy);
	mPostSSAPasses.push_back(new OptPassJumps);
	mPostSSAPasses.push_back(new OptPassDce);
}