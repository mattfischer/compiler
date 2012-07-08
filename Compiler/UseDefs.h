#ifndef USE_DEFS_H
#define USE_DEFS_H

#include "IR.h"
#include "ReachingDefs.h"

#include <map>
#include <set>
#include <vector>

class UseDefs
{
public:
	UseDefs(std::vector<IR::Block*> &blocks, ReachingDefs &reachingDefs);

	typedef std::set<IR::Entry*> EntrySet;
	EntrySet &uses(IR::Entry *def);
	EntrySet &defines(IR::Entry *use, IR::Symbol *symbol);
	void print();

private:
	typedef std::map<IR::Symbol*, EntrySet> SymbolToEntrySetMap;

	std::map<IR::Entry*, EntrySet> mUses;
	std::map<IR::Entry*, SymbolToEntrySetMap> mDefines;
	std::vector<IR::Block*> mBlocks;
};

#endif
