#ifndef ANALYSIS_USE_DEFS_H
#define ANALYSIS_USE_DEFS_H

#include <map>
#include <set>
#include <vector>

#include "IR/EntrySet.h"

namespace IR {
	class Block;
	class Entry;
	class Symbol;
}

namespace Analysis {
	class ReachingDefs;

	class UseDefs
	{
	public:
		UseDefs(const std::vector<IR::Block*> &blocks, const ReachingDefs &reachingDefs);

		const IR::EntrySet &uses(IR::Entry *def) const;
		const IR::EntrySet &defines(IR::Entry *use, IR::Symbol *symbol) const;

		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);
		void replaceUse(IR::Entry *entry, IR::Symbol *oldSymbol, IR::Symbol *newSymbol);
		void remove(IR::Entry *entry);

		void print() const;

	private:
		typedef std::map<IR::Symbol*, IR::EntrySet> SymbolToEntrySetMap;

		std::map<IR::Entry*, IR::EntrySet> mUses;
		std::map<IR::Entry*, SymbolToEntrySetMap> mDefines;
		std::vector<IR::Block*> mBlocks;
		const ReachingDefs &mReachingDefs;
	};
}
#endif
