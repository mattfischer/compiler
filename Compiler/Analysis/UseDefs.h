#ifndef ANALYSIS_USE_DEFS_H
#define ANALYSIS_USE_DEFS_H

#include <map>
#include <set>
#include <vector>

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

		typedef std::set<IR::Entry*> EntrySet;
		const EntrySet &uses(IR::Entry *def) const;
		const EntrySet &defines(IR::Entry *use, IR::Symbol *symbol) const;

		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);
		void remove(IR::Entry *entry);

		void removeUse(IR::Entry *entry, IR::Entry *use);
		void addUse(IR::Entry *entry, IR::Entry *use);

		void removeDefines(IR::Entry *entry, IR::Symbol *symbol);
		void addDefines(IR::Entry *entry, IR::Symbol *symbol, const EntrySet &defines);

		void print() const;

	private:
		typedef std::map<IR::Symbol*, EntrySet> SymbolToEntrySetMap;

		std::map<IR::Entry*, EntrySet> mUses;
		std::map<IR::Entry*, SymbolToEntrySetMap> mDefines;
		std::vector<IR::Block*> mBlocks;
	};
}
#endif
