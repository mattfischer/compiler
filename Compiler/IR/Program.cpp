#include "IR/Program.h"

#include "IR/Procedure.h"

namespace IR {
	Program::Program()
	{
		mMain = 0;
	}

	void Program::addProcedure(Procedure *procedure)
	{
		mProcedures.push_back(procedure);
		if(procedure->name() == "main") {
			mMain = procedure;
		}
	}


	Procedure *Program::findProcedure(const std::string &name)
	{
		for(ProcedureList::iterator it = mProcedures.begin(); it != mProcedures.end(); it++) {
			Procedure* procedure = *it;
			if(procedure->name() == name) {
				return procedure;
			}
		}

		return 0;
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