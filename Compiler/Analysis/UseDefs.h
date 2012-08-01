#ifndef ANALYSIS_USE_DEFS_H
#define ANALYSIS_USE_DEFS_H

#include <map>
#include <set>
#include <vector>

#include "IR/EntrySet.h"

#include "Analysis/ReachingDefs.h"

namespace IR {
	class Entry;
	class Symbol;
	class Procedure;
}

namespace Analysis {
	class UseDefs
	{
	public:
		UseDefs(IR::Procedure *procedure);

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
		IR::Procedure *mProcedure;
		ReachingDefs mReachingDefs;
	};
}
#endif
