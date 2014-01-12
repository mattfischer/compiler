#ifndef ANALYSIS_LIVE_VARIABLES_H
#define ANALYSIS_LIVE_VARIABLES_H

#include "IR/Procedure.h"
#include "IR/Symbol.h"
#include "IR/Entry.h"

#include <set>
#include <map>

namespace Analysis {
	class LiveVariables {
	public:
		LiveVariables(IR::Procedure *procedure);

		typedef std::set<IR::Symbol*> SymbolSet;
		SymbolSet &variables(IR::Entry *entry);
		void print();

	private:
		std::map<IR::Entry*, SymbolSet> mMap;
		IR::Procedure *mProcedure;
	};
}

#endif