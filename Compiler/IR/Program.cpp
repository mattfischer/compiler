#include "IR/Program.h"

#include "IR/Procedure.h"

namespace IR {
	Program::Program()
	{
		mMain = new Procedure("main");
		mProcedures.push_back(mMain);
	}

	void Program::print() const
	{
		for(unsigned int i=0; i<mProcedures.size(); i++) {
			printf("<%s>\n", mProcedures[i]->name().c_str());
			mProcedures[i]->print("  ");
		}
	}
}