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
		for(ProcedureList::const_iterator it = mProcedures.begin(); it != mProcedures.end(); it++) {
			Procedure *procedure = *it;
			printf("<%s>\n", procedure->name().c_str());
			procedure->print("  ");
		}
	}
}