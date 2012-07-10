#ifndef ANALYSIS_REACHING_DEFS_H
#define ANALYSIS_REACHING_DEFS_H

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

		typedef std::set<IR::Entry*> EntrySet;
		typedef std::map<IR::Symbol*, EntrySet> SymbolToEntrySetMap;
		const EntrySet &defs(IR::Entry* entry) const;
		const EntrySet defsForSymbol(IR::Entry* entry, IR::Symbol *symbol) const;
		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);
		void remove(IR::Entry *entry);
		void print() const;

	private:
		EntrySet createKillSet(const EntrySet &in, const EntrySet &gen);
		EntrySet createOutSet(const EntrySet &in, const EntrySet &gen, const EntrySet &kill);

		typedef std::map<IR::Entry*, EntrySet> EntryToEntrySetMap;
		FlowGraph &mFlowGraph;
		EntryToEntrySetMap mDefs;
	};
}
#endif