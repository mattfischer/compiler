#include "Middle/ErrorCheck.h"

#include "IR/Program.h"
#include "IR/Procedure.h"
#include "IR/Symbol.h"

#include "Analysis/LiveVariables.h"

namespace Middle {
	bool ErrorCheck::check(IR::Program *program)
	{
		for(IR::Program::ProcedureList::iterator it = program->procedures().begin(); it != program->procedures().end(); it++) {
			IR::Procedure *procedure = *it;

			Analysis::LiveVariables liveVariables(procedure);
			Analysis::LiveVariables::SymbolSet symbols = liveVariables.variables(procedure->entries().front());
			if(symbols.size() != 0) {
				IR::Symbol *symbol = *symbols.begin();
				printf("Error: Use of undefind variable %s in procedure %s\n", symbol->name.c_str(), procedure->name().c_str());
				return false;
			}
		}

		return true;
	}
}