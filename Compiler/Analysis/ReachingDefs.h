#ifndef ANALYSIS_REACHING_DEFS_H
#define ANALYSIS_REACHING_DEFS_H

#include "IR/EntrySet.h"

#include <vector>
#include <set>
#include <map>

namespace IR {
	class Block;
	class Entry;
	class Symbol;
}

namespace Analysis {
	class FlowGraph;

	class ReachingDefs {
	public:
		ReachingDefs(FlowGraph &flowGraph);

		typedef std::map<IR::Symbol*, IR::EntrySet> SymbolToEntrySetMap;
		const IR::EntrySet &defs(IR::Entry* entry) const;
		const IR::EntrySet defsForSymbol(IR::Entry* entry, IR::Symbol *symbol) const;
		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);
		void remove(IR::Entry *entry);
		void print() const;

	private:
		IR::EntrySet createKillSet(const IR::EntrySet &in, const IR::EntrySet &gen);
		IR::EntrySet createOutSet(const IR::EntrySet &in, const IR::EntrySet &gen, const IR::EntrySet &kill);

		typedef std::map<IR::Entry*, IR::EntrySet> EntryToEntrySetMap;
		FlowGraph &mFlowGraph;
		EntryToEntrySetMap mDefs;
	};
}
#endif