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

	std::set<IR::Entry*> &uses(IR::Entry *def);
	std::set<IR::Entry*> &defines(IR::Entry *use, IR::Symbol *symbol);
	void print();

private:
	std::map<IR::Entry*, std::set<IR::Entry*> > mUses;
	std::map<IR::Entry*, std::map<IR::Symbol*, std::set<IR::Entry*> > > mDefines;
	std::vector<IR::Block*> mBlocks;
};

#endif
