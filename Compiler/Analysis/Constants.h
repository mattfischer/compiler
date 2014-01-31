#ifndef ANALYSIS_CONSTANTS_H
#define ANALYSIS_CONSTANTS_H

#include "IR/Procedure.h"

#include "Analysis/UseDefs.h"

namespace Analysis {
	class Constants {
	public:
		Constants(IR::Procedure *procedure);

		int getValue(IR::Entry *entry, IR::Symbol *symbol, bool &isConstant);

	private:
		UseDefs mUseDefs;
	};
}

#endif