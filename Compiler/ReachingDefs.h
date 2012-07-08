#ifndef REACHING_DEFS_H
#define REACHING_DEFS_H

#include "IR.h"

#include <vector>
#include <set>
#include <map>

class ReachingDefs {
public:
	ReachingDefs(std::vector<IR::Block*> &blocks);

	typedef std::set<IR::Entry*> EntrySet;
	EntrySet &defs(IR::Entry* entry);
	void print();

private:
	EntrySet createKillSet(EntrySet &in, EntrySet &gen);
	EntrySet createOutSet(EntrySet &in, EntrySet &gen, EntrySet &kill);

	std::vector<IR::Block*> mBlocks;
	std::map<IR::Entry*, EntrySet> mDefs;
};
#endif