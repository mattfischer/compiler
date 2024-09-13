#ifndef ANALYSIS_CONSTANTS_H
#define ANALYSIS_CONSTANTS_H

#include "IR/Procedure.h"

#include "Analysis/UseDefs.h"

namespace Analysis {
	class Constants {
	public:
		Constants(const IR::Procedure &procedure, const UseDefs &useDefs);

		int getIntValue(const IR::Entry *entry, const IR::Symbol *symbol, bool &isConstant) const;
		std::string getStringValue(const IR::Entry *entry, const IR::Symbol *symbol, bool &isConstant) const;

	private:
		const UseDefs &mUseDefs;
	};
}

#endif