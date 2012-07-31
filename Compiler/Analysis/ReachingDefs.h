#ifndef ANALYSIS_REACHING_DEFS_H
#define ANALYSIS_REACHING_DEFS_H

#include "IR/EntrySet.h"

#include "Analysis/FlowGraph.h"

#include <vector>
#include <set>
#include <map>

namespace IR {
	class Entry;
	class Symbol;
	class Procedure;
}

namespace Analysis {
	class ReachingDefs {
	public:
		ReachingDefs(IR::Procedure *procedure);

		typedef std::map<IR::Symbol*, IR::EntrySet> SymbolToEntrySetMap;
		const IR::EntrySet &defs(IR::Entry* entry) const;
		const IR::EntrySet defsForSymbol(IR::Entry* entry, IR::Symbol *symbol) const;
		void print() const;

	private:
		typedef std::map<IR::Entry*, IR::EntrySet> EntryToEntrySetMap;

		FlowGraph mFlowGraph;
		EntryToEntrySetMap mDefs;
		IR::Procedure *mProcedure;
	};
}
#endif