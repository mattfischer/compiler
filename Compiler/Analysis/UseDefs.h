#ifndef ANALYSIS_USE_DEFS_H
#define ANALYSIS_USE_DEFS_H

#include "IR.h"
#include "ReachingDefs.h"

#include <map>
#include <set>
#include <vector>

namespace Analysis {
	class UseDefs
	{
	public:
		UseDefs(const std::vector<IR::Block*> &blocks, const ReachingDefs &reachingDefs);

		typedef std::set<IR::Entry*> EntrySet;
		const EntrySet &uses(IR::Entry *def) const;
		const EntrySet &defines(IR::Entry *use, IR::Symbol *symbol) const;
		void print() const;

	private:
		typedef std::map<IR::Symbol*, EntrySet> SymbolToEntrySetMap;

		std::map<IR::Entry*, EntrySet> mUses;
		std::map<IR::Entry*, SymbolToEntrySetMap> mDefines;
		std::vector<IR::Block*> mBlocks;
	};
}
#endif
