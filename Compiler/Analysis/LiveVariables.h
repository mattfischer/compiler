#ifndef ANALYSIS_LIVE_VARIABLES_H
#define ANALYSIS_LIVE_VARIABLES_H

#include <set>
#include <map>

namespace IR {
	class Procedure;
	class Symbol;
	class Entry;
}

namespace Analysis {
	class FlowGraph;

	class LiveVariables {
	public:
		LiveVariables(IR::Procedure *procedure, FlowGraph &graph);

		typedef std::set<IR::Symbol*> SymbolSet;
		SymbolSet &variables(IR::Entry *entry);
		void print();

	private:
		std::map<IR::Entry*, SymbolSet> mMap;
		IR::Procedure *mProcedure;
	};
}

#endif